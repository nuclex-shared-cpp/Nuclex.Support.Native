#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/TemporaryFileScope.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "Nuclex/Support/Text/StringConverter.h" // for StringConverter
#include "Platform/WindowsApi.h" // for WindowsApi
#include "Platform/WindowsFileApi.h" // for WindowsFileApi
#else
#include "Platform/LinuxFileApi.h" // for LinuxApi
#include "Platform/PosixApi.h" // for PosixApi

#include <unistd.h> // for ::write(), ::close(), ::unlink()
#include <cstdlib> // for ::getenv(), ::mkdtemp()
#endif

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Appends the user's/system's preferred temp directory to a path</summary>
  /// <param name="path">Path vector the temp directory will be appended to</param>
  void appendTempDirectory(std::wstring &path) {

    // Ask for the current users or for the system's temporary directory
    path.resize(MAX_PATH + 1);
    DWORD result = ::GetTempPathW(MAX_PATH + 1, path.data());
    if(unlikely(result == 0)) {
      DWORD errorCode = ::GetLastError();

      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not obtain path to temp directory", errorCode
      );
    }

    // Append the temporary directory to the provided string
    path.resize(result);

  }
#endif // defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Appends the user's/system's preferred temp directory to a path</summary>
  /// <param name="path">Path vector the temp directory will be appended to</param>
  void appendTempDirectory(std::vector<char> &path) {

    // Obtain the most likely system temp directory
    const char *tempDirectory = ::getenv(u8"TMPDIR");
    if(tempDirectory == nullptr) {
      tempDirectory = ::getenv(u8"TMP");
      if(tempDirectory == nullptr) {
        tempDirectory = ::getenv(u8"TEMP");
        if(tempDirectory == nullptr) {
          // This is safe (part of the file system standard and Linux standard base),
          // but we wanted to honor any possible user preferences first.
          tempDirectory = u8"/tmp";
        }
      }
    }

    // Append the temporary directory to the path vector
    while(*tempDirectory != 0) {
      path.push_back(*tempDirectory);
      ++tempDirectory;
    }

  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Creates a temporary file with a unique name on Windows systems</summary>
  /// <param name="path">Directory in which the temporary file will be created</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  std::wstring createTemporaryFile(const std::wstring &path, const std::string &prefix) {
    std::wstring fullPath;

    {
      std::wstring utf16NamePrefix = (
        Nuclex::Support::Text::StringConverter::WideFromUtf8(prefix)
      );

      // Call GetTempFileName() to let Windows sort out a unique file name
      fullPath.resize(MAX_PATH);
      UINT result = ::GetTempFileNameW(
        path.c_str(),
        utf16NamePrefix.c_str(),
        0, // let GetTempFileName() come up with a unique number
        fullPath.data()
      );
      // MSDN documents ERROR_BUFFER_OVERFLOW (111) as a possible return value but
      // that doesn't make any sense. Treating it as an error might introduce spurious
      // failures (a 1:65535 chance). Taking 111 out of the range of possible results
      // would be so weird that I feel safer assuming the docs are wrong.
      // (we're providing the maximum buffer size, though, so no overflow should ever happen)
      if(result == 0) {
        DWORD errorCode = ::GetLastError();

        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not acquire a unique temporary file name", errorCode
        );
      }

      // Truncate the MAX_PATH-sized string back to the actual number of characters
      std::string::size_type zeroTerminator = fullPath.find(L'\0');
      if(zeroTerminator == std::wstring::npos) {
        fullPath.resize(zeroTerminator);
      }
    }

    return fullPath;
  }
#endif // defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Builds the full template string that's passed to ::mkstemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMksTemp(std::vector<char> &path, const std::string &prefix) {
      path.reserve(256); // PATH_MAX would be a bit too bloaty usually...

    // Obtain the system's temporary directory (usually /tmp, can be overridden)
    //   path: "/tmp/"
    {
      appendTempDirectory(path);
      {
        std::string::size_type length = path.size();
        if(path[length -1] != '/') {
          path.push_back('/');
        }
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
      static const std::string placeholder(u8"XXXXXX", 6);
      path.insert(path.end(), placeholder.begin(), placeholder.end());
    }
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::TemporaryFileScope(const std::string &namePrefix /* = u8"tmp" */) :
    path(),
    privateImplementationData {0} {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    static_assert(
      (sizeof(this->privateImplementationData) >= sizeof(HANDLE)) &&
      u8"File handlefits in space provided for private implementation data"
    );
    *reinterpret_cast<HANDLE *>(this->privateImplementationData) = INVALID_HANDLE_VALUE;

    std::wstring temporaryDirectory;
    appendTempDirectory(temporaryDirectory);

    std::wstring fullPath = createTemporaryFile(temporaryDirectory, namePrefix);

    HANDLE fileHandle = ::CreateFileW(
      fullPath.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
      nullptr,
      CREATE_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not open temporary file '");
      errorMessage.append(Text::StringConverter::Utf8FromWide(fullPath));
      errorMessage.append(u8"' for writing");

      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    *reinterpret_cast<HANDLE *>(this->privateImplementationData) = fileHandle;
    this->path = Text::StringConverter::Utf8FromWide(fullPath);
#else
    static_assert(
      (sizeof(this->privateImplementationData) >= sizeof(int)) &&
      u8"File descriptor fits in space provided for private implementation data"
    );

    // Build the path template including the system's temporary directory
    std::vector<char> pathTemplate;
    buildTemplateForMksTemp(pathTemplate, namePrefix);

    // Select and open a unique temporary filename
    int fileDescriptor = ::mkstemp(pathTemplate.data());
    if(unlikely(fileDescriptor == -1)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not create temporary file '");
      errorMessage.append(pathTemplate.data(), pathTemplate.size());
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    // Store the file handle in the private implementation data block and
    // remember the full path for when the user queries it later
    *reinterpret_cast<int *>(this->privateImplementationData) = fileDescriptor;
    this->path.assign(pathTemplate.begin(), pathTemplate.end());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::~TemporaryFileScope() {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    HANDLE fileHandle = *reinterpret_cast<HANDLE *>(this->privateImplementationData);

    if(likely(fileHandle != INVALID_HANDLE_VALUE)) {
      BOOL result = ::CloseHandle(fileHandle);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Handle of temporary file is closed successfully");
    }

    if(likely(!this->path.empty())) {
      std::wstring utf16Path = Text::StringConverter::WideFromUtf8(this->path);
      BOOL result = ::DeleteFileW(utf16Path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary file is successfully deleted after use");
    }
#else
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);

    // Close the file so we don't leak handles
    if(likely(fileDescriptor != 0)) {
      int result = ::close(fileDescriptor);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is closed successfully");
    }

    // Delete the file. Even if the close failed, on Linux systems we
    // can delete the file and it will be removed from the file index
    // (and the data will disppear as soon as the last process closes it).
    if(likely(!this->path.empty())) {
      int result = ::unlink(this->path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is deleted successfully");
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryFileScope::SetFileContents(
    const std::uint8_t *contents, std::size_t byteCount
  ) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    HANDLE fileHandle = *reinterpret_cast<HANDLE *>(this->privateImplementationData);
    assert((fileHandle != INVALID_HANDLE_VALUE) && u8"File is opened and accessible");

    Platform::WindowsFileApi::Seek(fileHandle, 0, FILE_BEGIN);
    Platform::WindowsFileApi::Write(fileHandle, contents, byteCount);
    Platform::WindowsFileApi::SetLengthToFileCursor(fileHandle);
    Platform::WindowsFileApi::FlushFileBuffers(fileHandle);
#else
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);
    assert((fileDescriptor != 0) && u8"File is opened and accessible");

    Platform::LinuxFileApi::Seek(fileDescriptor, ::off_t(0), SEEK_SET);
    Platform::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
    Platform::LinuxFileApi::SetLength(fileDescriptor, byteCount);
    Platform::LinuxFileApi::Flush(fileDescriptor);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
