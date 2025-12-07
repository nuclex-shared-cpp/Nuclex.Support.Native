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
      Nuclex::Support::Platform::PosixPathApi::GetTemporaryDirectory(path);

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
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Callback for the file-tree-walk function that deletes files</summary>
  /// <param name="path">Path of the current file or directory</param>
  /// <param name="nodeStat">Stat structure of the current file or directory</param>
  /// <param name="typeflag">Type of the current node (FTW_F or FTW_D)</param>
  /// <param name="ftwinfo">Folder depth an other info provided by nftw()</param>
  /// <returns>0 on success, a standard Posix error code in case of failure</returns>
  int removeFileOrDirectoryCallback(
    const char *path, const struct ::stat *nodeStat, int typeflag, struct ::FTW *ftwinfo
  ) {
    (void)nodeStat;
    (void)typeflag;
    (void)ftwinfo;

    return ::remove(path);
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Recursively deletes the specified directory and all its contents</summary>
  /// <param name="path">Absolute path of the directory that will be deleted</param>
  /// <remarks>
  ///   The path must not be terminated with a path separator.
  /// </remarks>
  void deleteDirectoryWithContents(const std::wstring &path) {
    static const std::wstring allFilesMask(L"\\*");

    ::WIN32_FIND_DATAW findData;

    // First, delete the contents of the directory, recursively for subdirectories
    std::wstring searchMask = path + allFilesMask;
    HANDLE searchHandle = ::FindFirstFileExW(
      searchMask.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, 0
    );
    if(searchHandle == INVALID_HANDLE_VALUE) {
      DWORD lastError = ::GetLastError();
      if(lastError != ERROR_FILE_NOT_FOUND) { // or ERROR_NO_MORE_FILES, ERROR_NOT_FOUND?
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForFileSystemError(
          u8"Could not start directory enumeration", lastError
        );
      }
    }

    // Did this directory have any contents? If so, delete them first
    if(searchHandle != INVALID_HANDLE_VALUE) {
      ON_SCOPE_EXIT {
        BOOL result = ::FindClose(searchHandle);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result != FALSE) && u8"Search handle is closed successfully");
      };
      for(;;) {

        // Do not process the obligatory '.' and '..' directories
        if(findData.cFileName[0] != '.') {
          bool isDirectory = (
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
          );

          // Subdirectories need to be handled by deleting their contents first
          std::wstring filePath = path + L'\\' + findData.cFileName;
          if(isDirectory) {
            deleteDirectoryWithContents(filePath);
          } else {
            BOOL result = ::DeleteFileW(filePath.c_str());
            if(result == FALSE) {
              DWORD lastError = ::GetLastError();
              Nuclex::Support::Platform::WindowsApi::ThrowExceptionForFileSystemError(
                u8"Could not delete temporary file", lastError
              );
            }
          }
        }

        // Advance to the next file in the directory
        BOOL result = ::FindNextFileW(searchHandle, &findData);
        if(result == FALSE) {
          DWORD lastError = ::GetLastError();
          if(lastError != ERROR_NO_MORE_FILES) {
            Nuclex::Support::Platform::WindowsApi::ThrowExceptionForFileSystemError(
              u8"Error during directory enumeration", lastError
            );
          }
          break; // All directory contents enumerated and deleted
        }

      } // for
    }

    // The directory is empty, we can now safely remove it
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if(result == FALSE) {
      DWORD lastError = ::GetLastError();
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForFileSystemError(
        u8"Could not remove nested temporary directory", lastError
      );
    }
  }
#endif // defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::TemporaryDirectoryScope(
    const std::u8string &namePrefix /* = u8"tmp" */
  ) : path() {
#if defined(NUCLEX_SUPPORT_WINDOWS)

    // Ask Windows to create a unique temporary file for us
    std::wstring filePath = Platform::WindowsPathApi::CreateTemporaryFile(namePrefix);
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

      Platform::WindowsApi::ThrowExceptionForFileSystemError(errorMessage, errorCode);
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

      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    // Store the full path to the temporary directory we just created
    assert((path == pathTemplateChars.c_str()) && u8"Original path buffer is modified");
    this->path.assign(pathTemplateChars);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::~TemporaryDirectoryScope() {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring widePath;
    Text::StringConverter::AppendPathAsWide(widePath, this->path);
    deleteDirectoryWithContents(widePath);

    std::wstring::size_type length = widePath.length();
    if(length > 4) {
      widePath.resize(length - 4);
      BOOL result = ::DeleteFileW(widePath.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary directory scope file is deleted successfully");
    }
#else
    int result = ::nftw(
      this->path.c_str(), removeFileOrDirectoryCallback, 64, FTW_DEPTH | FTW_PHYS
    );
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not erase temporary directory contents in '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, this->path);
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
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
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForWriting(filePath);
      ON_SCOPE_EXIT {
        Platform::WindowsFileApi::CloseFile<Platform::ErrorPolicy::Assert>(fileHandle);
      };

      Platform::WindowsFileApi::Write(fileHandle, contents, byteCount);
      Platform::WindowsFileApi::FlushFileBuffers(fileHandle);
    }
#elif defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForWriting(filePath);
      ON_SCOPE_EXIT {
        Platform::LinuxFileApi::Close(fileDescriptor);
      };

      Platform::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
      Platform::LinuxFileApi::Flush(fileDescriptor);
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
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Platform::WindowsFileApi::CloseFile(fileHandle); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Platform::WindowsFileApi::Read(
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
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
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
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT {
        Platform::WindowsFileApi::CloseFile<Platform::ErrorPolicy::Assert>(fileHandle);
      };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::uint8_t *data = reinterpret_cast<std::uint8_t *>(contents.data());
        std::size_t readByteCount = Platform::WindowsFileApi::Read(
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
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(filePath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::byte *data = reinterpret_cast<std::byte *>(contents.data());
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
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
