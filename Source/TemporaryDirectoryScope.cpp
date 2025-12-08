#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/TemporaryDirectoryScope.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "Interop/WindowsApi.h" // for WindowsApi
#include "Interop/WindowsPathApi.h" // for WindowsPathApi
#include "Interop/WindowsFileApi.h" // for WindowsFileApi
#else
#include "Interop/PosixApi.h" // for PosixApi
#include "Interop/PosixPathApi.h" // for PosixPathApi
#include "Interop/LinuxFileApi.h" // for LinuxFileApi
#include "Interop/PosixFileApi.h" // for PosixFileApi

#include <ftw.h> // for struct ::ftw
#include <sys/stat.h> // for struct ::stat
#include <unistd.h> // for ::write(), ::close(), ::unlink()
#include <cstdlib> // for ::getenv(), ::mkdtemp()
#endif

#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Builds the full template string that's passed to ::mkdtemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMkdTemp(std::u8string &path, const std::u8string &prefix) {
    path.reserve(256); // PATH_MAX would be a bit too bloaty usually...

    // Obtain the system's temporary directory (usually /tmp, can be overridden)
    //   path: "/tmp/"
    {
      Nuclex::Support::Interop::PosixPathApi::GetTemporaryDirectory(path);

      std::u8string::size_type length = path.size();
      if(path[length -1] != '/') {
        path.push_back('/');
      }
    }

    // Append the user-specified prefix, if any
    //   path: "/tmp/myapp"
    if(!prefix.empty()) {
      path.insert(path.end(), prefix.begin(), prefix.end());
    }

    // Append the mandatory placeholder characters
    //   path: "/tmp/myappXXXXXX"
    {
      static const std::u8string placeholder(u8"XXXXXX", 6);
      path.insert(path.end(), placeholder.begin(), placeholder.end());
    }
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void deleteDirectoryContents(const std::filesystem::path &path) {
    using Nuclex::Support::Interop::WindowsFileApi;
    using Nuclex::Support::Interop::ErrorPolicy;

    ::WIN32_FIND_DATAW findData;

    // Enumerate the contents of the immediate directory in the path given to us
    HANDLE searchHandle = WindowsFileApi::FindFirstFile<ErrorPolicy::Assert>(
      path / L"*", findData
    );
    if(searchHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
      return;
    }

    // The directory has contents so we have to delete them now
    {
      ON_SCOPE_EXIT {
        WindowsFileApi::CloseSearch<ErrorPolicy::Assert>(searchHandle);
      };
      for(;;) {

        // Do not process the obligatory '.' and '..' directories
        bool isSelfOrParent = (
          (findData.cFileName[0] == L'.') &&
          (
            (findData.cFileName[1] == L'\0') ||
            (
              (findData.cFileName[1] == L'.') &&
              (findData.cFileName[2] == L'\0')
            )
          )
        );
        if(!isSelfOrParent) [[likely]] {
          bool isDirectory = (
            //((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) ||
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
          );

          // Subdirectories need to be handled by deleting their contents first
          std::filesystem::path itemPath = path / findData.cFileName;
          if(isDirectory) {
            deleteDirectoryContents(itemPath);
            WindowsFileApi::DeleteDirectory<ErrorPolicy::Assert>(itemPath);
          } else {
            WindowsFileApi::DeleteFile<ErrorPolicy::Assert>(itemPath);
          }
        } // if current entry isn't the self or parent directory link

        // Advance to the next file in the directory
        bool reachedNextFile = WindowsFileApi::FindNextFile<ErrorPolicy::Assert>(
          searchHandle, findData
        );
        if(!reachedNextFile) [[unlikely]] {
          break; // All directory contents enumerated and deleted
        }
      } // for ever
    } // beauty scope
  }
#endif // defined(NUCLEX_TEMPCLEANER_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  void deleteDirectoryContents(const std::filesystem::path &path) {
    using Nuclex::Support::Interop::LinuxFileApi;
    using Nuclex::Support::Interop::PosixFileApi;
    using Nuclex::Support::Interop::ErrorPolicy;

    // Enumerate the contents of the immediate directory in the path given to us
    ::DIR *directory = PosixFileApi::OpenDirectory<ErrorPolicy::Assert>(path);
    if(directory == nullptr) {
      return;
    }

    // The directory has contents so we have to delete them now
    {
      ON_SCOPE_EXIT {
        PosixFileApi::CloseDirectory<ErrorPolicy::Assert>(directory);
      };
      for(;;) {
        const struct ::dirent *entry = PosixFileApi::ReadDirectory<ErrorPolicy::Assert>(directory);
        if(entry == nullptr) {
          break; // an empty entry signals the end of the enumeration
        }

        // Do not process the obligatory '.' and '..' directories
        bool isSelfOrParent = (
          (entry->d_name[0] == '.') &&
          (
            (entry->d_name[1] == '\0') ||
            (
              (entry->d_name[1] == '.') &&
              (entry->d_name[2] == '\0')
            )
          )
        );
        if(!isSelfOrParent) [[likely]] {
          std::filesystem::path itemPath = path / entry->d_name;

          // While entry->d_type could tell us if it's a file or a directory (unless it's
          // DT_UNKNOWN on some file systems that don't support it), we still need
          // the file size, which forces us to call ::stat() either way.
          struct ::stat fileStatus;
          bool isAccessible = PosixFileApi::LStat<ErrorPolicy::Assert>(itemPath, fileStatus);
          if(isAccessible) [[likely]] {
            if(S_ISDIR(fileStatus.st_mode)) {
              deleteDirectoryContents(itemPath);
              PosixFileApi::RemoveDirectory<ErrorPolicy::Assert>(itemPath);
            } else {
              PosixFileApi::RemoveFile<ErrorPolicy::Assert>(itemPath);
            }
          }
        } // if current entry is not the current or directory link
      } // for ever (until all items are enumerated)
    } // directory enumeration closing scope
  }
#endif // !defined(NUCLEX_TEMPCLEANER_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::TemporaryDirectoryScope(
    const std::u8string &namePrefix /* = u8"tmp" */
  ) : path() {
#if defined(NUCLEX_SUPPORT_WINDOWS)

    // Ask Windows to create a unique temporary file for us
    std::wstring filePath = Interop::WindowsPathApi::CreateTemporaryFile(namePrefix);
    auto temporaryFileDeleter = ON_SCOPE_EXIT_TRANSACTION {
      BOOL result = ::DeleteFileW(filePath.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary file is deleted successfully in error handler");
    };

    // Leave the file in place and add a '.dir' to the end for the temporary directory
    std::wstring directoryPath;
    directoryPath.reserve(filePath.length() + 5);
    directoryPath.assign(filePath);
    static const std::wstring suffix(L".dir", 4);
    directoryPath.append(suffix);

    // Create the temporary directory
    BOOL result = ::CreateDirectoryW(directoryPath.c_str(), nullptr);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      std::u8string errorMessage(u8"Could not create directory '");
      errorMessage.append(Text::StringConverter::Utf8FromWide(directoryPath));
      errorMessage.append(u8"'");

      Interop::WindowsApi::ThrowExceptionForFileSystemError(errorMessage, errorCode);
    }

    // Everything worked out, remember the path and disarm the scope guard
    temporaryFileDeleter.Commit();
    this->path.assign(Text::StringConverter::Utf8FromWide(directoryPath));

#else

    // Build a template string in the system's temp directory to call ::mkdtemp()
    std::u8string pathTemplate;
    buildTemplateForMkdTemp(pathTemplate, namePrefix);

    // Select and open a unique temporary directory name
    std::string pathTemplateChars(pathTemplate.begin(), pathTemplate.end());
    const char *path = ::mkdtemp(pathTemplateChars.data());
    if(path == nullptr) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not create temporary directory '");
      errorMessage.append(pathTemplate);
      errorMessage.append(u8"'");

      Interop::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    // Store the full path to the temporary directory we just created
    assert((path == pathTemplateChars.c_str()) && u8"Original path buffer is modified");
    this->path.assign(pathTemplateChars);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::~TemporaryDirectoryScope() {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    deleteDirectoryContents(this->path);
    Interop::WindowsFileApi::DeleteDirectory(this->path);

    std::wstring widePath = this->path.wstring();
    std::wstring::size_type length = widePath.length();
    if(length > 4) {
      widePath.resize(length - 4);
      Interop::WindowsFileApi::DeleteFile<
        Interop::ErrorPolicy::Assert
      >(widePath);
    }
#else
    deleteDirectoryContents(this->path);
    Interop::PosixFileApi::RemoveDirectory(this->path);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::filesystem::path TemporaryDirectoryScope::GetPath(const std::u8string &filename) const {
    return (this->path / filename);
  }

  // ------------------------------------------------------------------------------------------- //

  std::filesystem::path TemporaryDirectoryScope::PlaceFile(
    const std::u8string &filename, const std::byte *contents, std::size_t byteCount
  ) {
    std::filesystem::path filePath = GetPath(filename);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Interop::WindowsFileApi::OpenFileForWriting(filePath);
      ON_SCOPE_EXIT {
        Interop::WindowsFileApi::CloseFile<Interop::ErrorPolicy::Assert>(fileHandle);
      };

      Interop::WindowsFileApi::Write(fileHandle, contents, byteCount);
      Interop::WindowsFileApi::FlushFileBuffers(fileHandle);
    }
#elif defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Interop::LinuxFileApi::OpenFileForWriting(filePath);
      ON_SCOPE_EXIT {
        Interop::LinuxFileApi::Close(fileDescriptor);
      };

      Interop::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
      Interop::LinuxFileApi::Flush(fileDescriptor);
    }
#endif

    return filePath;
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryDirectoryScope::ReadFile(
    const std::u8string &filename, std::vector<std::byte> &contents
  ) const {
    std::filesystem::path filePath = GetPath(filename);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Interop::WindowsFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Interop::WindowsFileApi::CloseFile(fileHandle); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Interop::WindowsFileApi::Read(
          fileHandle, contents.data() + offset, 4096
        );
        offset += readByteCount;
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#elif defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Interop::LinuxFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Interop::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Interop::LinuxFileApi::Read(
          fileDescriptor, contents.data() + offset, 4096
        );
        offset += readByteCount;
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryDirectoryScope::ReadFile(
    const std::u8string &filename, std::u8string &contents
  ) const {
    std::filesystem::path filePath = GetPath(filename);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Interop::WindowsFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT {
        Interop::WindowsFileApi::CloseFile<Interop::ErrorPolicy::Assert>(fileHandle);
      };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::uint8_t *data = reinterpret_cast<std::uint8_t *>(contents.data());
        std::size_t readByteCount = Interop::WindowsFileApi::Read(
          fileHandle, data + offset, 4096
        );
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          offset += readByteCount;
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#elif defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Interop::LinuxFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Interop::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::byte *data = reinterpret_cast<std::byte *>(contents.data());
        std::size_t readByteCount = Interop::LinuxFileApi::Read(
          fileDescriptor, data + offset, 4096
        );
        offset += readByteCount;
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support
