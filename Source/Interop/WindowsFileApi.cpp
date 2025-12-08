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

#include "WindowsFileApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/UnicodeHelper.h" // for UTF-16 <-> UTF-8 conversion
#include "Nuclex/Support/Text/StringConverter.h" // for StringConverter

#include <stdexcept> // for std::invalid_argument

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw std::invalid_argument(
        reinterpret_cast<const char *>(
          u8"Illegal UTF-8 character(s) encountered"
        )
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a UTF-8 path into a UTF-16 path</summary>
  /// <param name="path">String containing a UTF-8 path</param>
  /// <returns>The UTF-16 path with magic prefix to eliminate the path length limit</returns>
  std::wstring wideFromPath(const std::filesystem::path &path) {
    if(path.empty()) {
      return std::wstring();
    }

    // UNC path format
    //
    // https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd
    // https://docs.microsoft.com/en-us/dotnet/standard/io/file-path-formats#example-ways-to-refer-to-the-same-file
    //
    // Valid inputs
    //   - file.txt                      -> \\?\file.txt
    //   - D:/dir/file.txt               -> \\?\D:\dir\file.txt
    //   - \\Server\share\file.txt       -> \\?\UNC\Server\share\file.txt
    //   - \\?\D:\file.txt               -> (keep)
    //   - \\?\UNC\Server\share\file.txt -> (keep)
    //   - \\.\D:\file.txt               -> (keep) // because the user may have their reasons
    //   - \\.\UNC\Server\file.txt       -> (keep) // because the user may have their reasons
    //
    // Note that this renders relative paths (..\) unusable.
    //

    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most file names will only use ascii characters.
    std::wstring widePath;
    {
      const std::filesystem::path::string_type &pathString = path.native();

      if(path.is_absolute()) {
        widePath.reserve(pathString.length() + 4);

        // According to Microsoft, this is how you lift the 260 char MAX_PATH limit.
        // Also skips the internal call to GetFullPathName() every API method does
        // internally, so paths have to be absolute.
        // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
        const wchar_t prefix[] = L"\\\\?\\";
        widePath.push_back(prefix[0]);
        widePath.push_back(prefix[1]);
        widePath.push_back(prefix[2]);
        widePath.push_back(prefix[3]);
      } else {
        widePath.reserve(pathString.length() + 1);
      }

      // If the string already is UTF-16 (likely when compiling in MSVC),
      // we can directly copy it over, only fixing forward slashes
      if constexpr(std::is_same<std::filesystem::path::string_type, std::wstring>::value) {
        const std::filesystem::path::string_type::value_type *start = pathString.data();
        const std::filesystem::path::string_type::value_type *end = start + pathString.length();
        while(start < end) {
          if(*start == L'/') {
            widePath.push_back(L'\\');
          } else {
            widePath.push_back(*start);
          }
          ++start;
        }
      } else { // Nope, path has another format, convert to UTF-16 and fix slashes
        std::u16string utf16PathString(path.u16string());
        const std::u16string::value_type *start = utf16PathString.data();
        const std::u16string::value_type *end = start + utf16PathString.length();
        while(start < end) {
          if(*start == L'/') {
            widePath.push_back(L'\\');
          } else {
            widePath.push_back(*start);
          }
          ++start;
        }
      }
    }

    return widePath;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileApi::OpenFileForReading(const std::filesystem::path &path) {
    HANDLE fileHandle = ::CreateFileW(
      path.c_str(),
      GENERIC_READ, // desired access
      FILE_SHARE_READ, // share mode,
      nullptr,
      OPEN_EXISTING, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      std::u8string errorMessage(u8"Could not open file '");
      errorMessage.append(path.u8string());
      errorMessage.append(u8"' for reading");

      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileApi::OpenFileForWriting(const std::filesystem::path &path) {
    std::wstring widePath = wideFromPath(path);

    HANDLE fileHandle = ::CreateFileW(
      path.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      0, // share mode
      nullptr,
      OPEN_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      std::u8string errorMessage(u8"Could not open file '");
      errorMessage.append(path.u8string());
      errorMessage.append(u8"' for writing");

      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileApi::OpenFileForSharedWriting(const std::filesystem::path &path) {
    HANDLE fileHandle = ::CreateFileW(
      path.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
      nullptr,
      OPEN_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      std::u8string errorMessage(u8"Could not open file '");
      errorMessage.append(path.u8string());
      errorMessage.append(u8"' for writing");

      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Seek(HANDLE fileHandle, std::ptrdiff_t offset, DWORD anchor) {
    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = offset;
    LARGE_INTEGER newFilePointer;

    BOOL result = ::SetFilePointerEx(
      fileHandle, distanceToMove, &newFilePointer, anchor
    );
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not move file cursor");
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(newFilePointer.QuadPart);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Read(HANDLE fileHandle, void *buffer, std::size_t count) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::ReadFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not read data from file");
      Interop::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Write(
    HANDLE fileHandle, const void *buffer, std::size_t count
  ) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::WriteFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not write data from file");
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::SetLengthToFileCursor(HANDLE fileHandle) {
    BOOL result = ::SetEndOfFile(fileHandle);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not truncate/pad file to file cursor position");
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::FlushFileBuffers(HANDLE fileHandle) {
    BOOL result = ::FlushFileBuffers(fileHandle);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not flush file buffers");
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WindowsFileApi::CloseFile<ErrorPolicy::Throw>(HANDLE fileHandle) {
    BOOL result = ::CloseHandle(fileHandle);
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      std::u8string errorMessage(u8"Could not close file handle");
      WindowsApi::ThrowExceptionForFileSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WindowsFileApi::CloseFile<ErrorPolicy::Assert>(HANDLE fileHandle) {
    BOOL result = ::CloseHandle(fileHandle);
    assert((result != FALSE) && u8"File must be closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool WindowsFileApi::DeleteDirectory<ErrorPolicy::Throw>(
    const std::filesystem::path &path
  ) {
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      // If the diectory is missing, we treat it as a successful outcome since the effect
      // desired by the caller, the directory being gone / not there, is achieved automatically.
      bool isMissingFileError = (
        (errorCode == ERROR_PATH_NOT_FOUND) ||
        (errorCode == ERROR_FILE_NOT_FOUND)
      );
      if(isMissingFileError) {
        return true;
      }

      // This is not one of the acceptable errors, so throw an exception
      std::u8string errorMessage(u8"Could not delete directory '");
      errorMessage.append(path.u8string());
      errorMessage.push_back(u8'\'');
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool WindowsFileApi::DeleteDirectory<ErrorPolicy::Assert>(
    const std::filesystem::path &path
  ) {
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      // If the diectory is missing, we treat it as a successful outcome since the effect
      // desired by the caller, the directory being gone / not there, is achieved automatically.
      bool isMissingFileError = (
        (errorCode == ERROR_PATH_NOT_FOUND) ||
        (errorCode == ERROR_FILE_NOT_FOUND)
      );
      if(isMissingFileError) {
        return true;
      }

      assert((result != FALSE) && u8"Directory should be deleted successfully");
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool WindowsFileApi::DeleteFile<ErrorPolicy::Throw>(
    const std::filesystem::path &path
  ) {
    BOOL result = ::DeleteFileW(path.c_str());
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      // If the file is missing, we treat it as a successful outcome since the effect
      // desired by the caller, the file being gone / not there, is achieved automatically.
      bool isMissingFileError = (
        (errorCode == ERROR_PATH_NOT_FOUND) ||
        (errorCode == ERROR_FILE_NOT_FOUND)
      );
      if(isMissingFileError) {
        return true;
      }

      // This is not one of the acceptable errors, so throw an exception
      std::u8string errorMessage(u8"Could not delete file  '");
      errorMessage.append(path.u8string());
      errorMessage.push_back(u8'\'');
      WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool WindowsFileApi::DeleteFile<ErrorPolicy::Assert>(
    const std::filesystem::path &path
  ) {
    BOOL result = ::DeleteFileW(path.c_str());
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();

      // If the file is missing, we treat it as a successful outcome since the effect
      // desired by the caller, the file being gone / not there, is achieved automatically.
      bool isMissingFileError = (
        (errorCode == ERROR_PATH_NOT_FOUND) ||
        (errorCode == ERROR_FILE_NOT_FOUND)
      );
      if(isMissingFileError) {
        return true;
      }

      assert((result != FALSE) && u8"File should be deleted successfully");
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
