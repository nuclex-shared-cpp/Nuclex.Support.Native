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

#include "WindowsFileAccessApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"
#include "../Text/Utf8/checked.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a UTF-8 path into a UTF-16 path</summary>
  /// <param name="utf8Path">String containing a UTF-8 path</param>
  /// <returns>The UTF-16 path with magic prefix to eliminate the path length limit</returns>
  std::wstring utf16FromUtf8Path(const std::string &utf8Path) {
    if(utf8Path.empty()) {
      return std::wstring();
    }

    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::wstring utf16Path;
    utf16Path.reserve(utf8Path.length() + 4);

    // According to Microsoft, this is how you lift the 260 char MAX_PATH limit.
    // Also skips the internal call to GetFullPathName() every API method does internally,
    // so paths have to be normalized and 
    const wchar_t prefix[] = L"\\\\?\\";
    utf16Path.push_back(prefix[0]);
    utf16Path.push_back(prefix[1]);
    utf16Path.push_back(prefix[2]);
    utf16Path.push_back(prefix[3]);

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to16(utf8Path.begin(), utf8Path.end(), std::back_inserter(utf16Path));

    return utf16Path;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileAccessApi::OpenFileForReading(const std::string &path) {
    std::wstring utf16Path = utf16FromUtf8Path(path);

    HANDLE fileHandle = ::CreateFileW(
      utf16Path.c_str(),
      GENERIC_READ, // desired access
      FILE_SHARE_READ, // share mode,
      nullptr,
      OPEN_EXISTING, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileAccessApi::OpenFileForWriting(const std::string &path) {
    std::wstring utf16Path = utf16FromUtf8Path(path);

    HANDLE fileHandle = ::CreateFileW(
      utf16Path.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      0, // share mode
      nullptr,
      OPEN_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileAccessApi::Read(HANDLE fileHandle, void *buffer, std::size_t count) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::ReadFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not read data from file");
      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileAccessApi::Write(
    HANDLE fileHandle, const void *buffer, std::size_t count
  ) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::WriteFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not write data from file");
      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileAccessApi::FlushFileBuffers(HANDLE fileHandle) {
    BOOL result = ::FlushFileBuffers(fileHandle);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not flush file buffers");
      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileAccessApi::CloseFile(HANDLE fileHandle, bool throwOnError /* = true */) {
    BOOL result = ::CloseHandle(fileHandle);
    if(throwOnError && (result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not close file handle");
      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Helpers

#endif // defined(NUCLEX_SUPPORT_WINDOWS)