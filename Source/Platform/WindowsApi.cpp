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

#include "WindowsApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Errors/FileAccessError.h" // for FileAccessError

#include "Nuclex/Support/Text/StringConverter.h" // to convert UTF-16 wholesome
#include "Nuclex/Support/Text/ParserHelper.h" // to skip trailing whitespace
#include "Nuclex/Support/Text/LexicalAppend.h" // to append error codes to strings
#include "Nuclex/Support/Text/UnicodeHelper.h" // for UTF-16 <-> UTF-8 conversion

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Releases memory that has been allocated by LocalAlloc()</summary>
  class LocalAllocScope {

    /// <summary>Initializes a new local memory releaser for the specified pointer</summary>
    /// <param name="localAddress">Pointer that will be released</param>
    public: LocalAllocScope(void *localAddress) : localAddress(localAddress) {}

    /// <summary>Frees the memory the memory releaser is responsible for</summary>
    public: ~LocalAllocScope() {
      ::LocalFree(localAddress);
    }

    /// <summary>Pointer to the memory the memory releaser will release</summary>
    private: void *localAddress;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Searches a string for a zero terminator and truncates everything after</summary>
  /// <param name="stringToTrim">String that will be trimmed</param>
  void trimStringToZeroTerminator(std::wstring &stringToTrim) {
    std::wstring::size_type terminatorIndex = stringToTrim.find(L'\0');
    if(terminatorIndex != std::wstring::npos) {
      stringToTrim.resize(terminatorIndex);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(int errorNumber) {
    std::wstring buffer(256, L'\0');
    for(;;) {

      // Try to obtain the error message relating to the POSIX error number. In order to be
      // unicode-safe, we have to use Microsoft's UTF-16 methods.
      errno = 0;
      int lookupErrorNumber = ::_wcserror_s(buffer.data(), buffer.length(), errorNumber);
      if(lookupErrorNumber == 0) {
        int errorNumberFromStrError = errno;
        if(errorNumberFromStrError == 0) {
          trimStringToZeroTerminator(buffer);
          return Nuclex::Support::Text::StringConverter::Utf8FromWide(buffer);
        }

        // If the buffer was too small, try again with 1024 bytes, 4096 bytes and
        // 16384 bytes, then blow up.
        if(errorNumberFromStrError == ERANGE) {
          std::size_t bufferSize = buffer.length();
          if(bufferSize < 16384) {
            buffer.resize(bufferSize * 4);
            continue;
          }
        }
      }

      // We failed to look up the error message. At least output the original
      // error number and remark that we weren't able to look up the error message.
      std::string errorMessage(u8"Error ");
      Text::lexical_append(errorMessage, errorNumber);
      errorMessage.append(u8" (and error message lookup failed)");
      return errorMessage;

    } // for(;;)
  }

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(DWORD errorCode) {

    // Use FormatMessage() to ask Windows for a human-readable error message
    LPWSTR errorMessageBuffer;
    DWORD errorMessageLength = ::FormatMessageW(
      (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS
      ),
      nullptr, // message string, ignored with passed flags
      errorCode,
      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Try for an english message first
      reinterpret_cast<LPWSTR>(&errorMessageBuffer), // MS wants us to cast off pointer levels!
      0,
      nullptr
    );
    if(errorMessageLength == 0) {
      // MSDN states that "Last-Error" will be set to ERROR_RESOURCE_LANG_NOT_FOUND,
      // but that doesn't really happen, so we recheck on any FormatMessage() failure
      errorMessageLength = ::FormatMessageW(
        (
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS
        ),
        nullptr, // message string, ignored with passed flags
        errorCode,
        0, // Let FormatMessage search: neutral, thread locale, user locale and system locale
        reinterpret_cast<LPWSTR>(&errorMessageBuffer), // MS wants us to cast off pointer levels!
        0,
        nullptr
      );
      if(errorMessageLength == 0) {
        std::string message(u8"Windows API error ");
        Text::lexical_append(message, static_cast<std::uint32_t>(errorCode));
        return message;
      }
    }

    // We don't want UTF-16 anywhere - at all. So convert this mess to UTF-8.
    std::string utf8ErrorMessage;
    {
      using Nuclex::Support::Text::UnicodeHelper;
      typedef UnicodeHelper::Char8Type Char8Type;

      LocalAllocScope errorMessageScope(errorMessageBuffer);

      utf8ErrorMessage.resize(errorMessageLength);
      {
        const char16_t *current = reinterpret_cast<const char16_t *>(errorMessageBuffer);
        const char16_t *end = current + errorMessageLength;
        Char8Type *write = reinterpret_cast<Char8Type *>(utf8ErrorMessage.data());
        while(current < end) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
          if(codePoint == char32_t(-1)) {
            break;
          }
          UnicodeHelper::WriteCodePoint(write, codePoint);
        }

        utf8ErrorMessage.resize(
          write - reinterpret_cast<Char8Type *>(utf8ErrorMessage.data())
        );
      }
    }

    // Microsoft likes to end their error messages with various spaces and newlines,
    // cut these off so we have a single-line error message
    std::string::size_type length = utf8ErrorMessage.length();
    while(length > 0) {
      if(!Text::ParserHelper::IsWhitespace(static_cast<char>(utf8ErrorMessage[length - 1]))) {
        break;
      }
      --length;
    }

    // If the error message is empty, return a generic one
    if(length == 0) {
      std::string message(u8"Windows API error ");
      Text::lexical_append(message, static_cast<std::uint32_t>(errorCode));
      return message;
    } else { // Error message had content, return it
      utf8ErrorMessage.resize(length);
      return utf8ErrorMessage;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(HRESULT resultHandle) {

    // The _com_error class has a bit of special code when the error message could
    // not be looked up. If the error code is greater than or equal to
    // WCODE_HRESULT_FIRST and also less than or equal to WCODE_HRESULT_LAST,
    // the error is a dispatch error (IDispatch, late-binding).
    //
    //     return (hr >= WCODE_HRESULT_FIRST && hr <= WCODE_HRESULT_LAST)
    //         ? WORD(hr - WCODE_HRESULT_FIRST)
    //         : 0;
    //
    // I don't think we'll encounter IDispatch errors in this library.
    return GetErrorMessage(static_cast<DWORD>(resultHandle));

  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForSystemError(
    const std::string &errorMessage, DWORD errorCode
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(errorCode));

    throw std::system_error(
      std::error_code(errorCode, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForFileSystemError(
    const std::string &errorMessage, DWORD errorCode
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(errorCode));

    // This is a good demonstration for why error codes are such a nasty mess...
    bool isFileAccessError = (
      (errorCode == ERROR_FILE_NOT_FOUND) || // file not found
      (errorCode == ERROR_PATH_NOT_FOUND) || // path not found
      (errorCode == ERROR_ACCESS_DENIED) || // access denied
      (errorCode == ERROR_FILE_READ_ONLY) || // file is read-only
      (errorCode == ERROR_INVALID_DRIVE) || // drive is invalid
      (errorCode == ERROR_CURRENT_DIRECTORY) || // current directory cannot be removed
      (errorCode == ERROR_NOT_SAME_DEVICE) || // file cannot be moved to a different disk
      (errorCode == ERROR_WRITE_PROTECT) || // medium is write protected
      (errorCode == ERROR_NOT_READY) || // device is not ready
      (errorCode == ERROR_CRC) || // data checksum error
      (errorCode == ERROR_SEEK) || // track or area cannot be located
      (errorCode == ERROR_NOT_DOS_DISK) || // wrong or unknown file system
      (errorCode == ERROR_SECTOR_NOT_FOUND) || // sector cannot be accessed
      (errorCode == ERROR_WRITE_FAULT) || // cannot write to the specified device
      (errorCode == ERROR_READ_FAULT) || // cannot read from the specified device
      (errorCode == ERROR_SHARING_VIOLATION) || // file is being accessed by another process
      (errorCode == ERROR_LOCK_VIOLATION) || // another proces has locked the file
      (errorCode == ERROR_HANDLE_EOF) || // too many file handles
      (errorCode == ERROR_HANDLE_DISK_FULL) || // disk is full
      (errorCode == ERROR_BAD_NETPATH) || // invalid network path
      (errorCode == ERROR_DEV_NOT_EXIST) || // device doesn't exist
      (errorCode == ERROR_DISK_CHANGE) || // wrong diskette inserted
      (errorCode == ERROR_DRIVE_LOCKED) || // drive is locked by another process
      (errorCode == ERROR_OPEN_FAILED) || // system cannot open the file
      (errorCode == ERROR_DISK_FULL) || // disk is full
      (errorCode == ERROR_NEGATIVE_SEEK) || // seek offset invalid
      (errorCode == ERROR_SEEK_ON_DEVICE) || // seeking not supported
      (errorCode == ERROR_BUSY_DRIVE) || // drive is busy
      (errorCode == ERROR_SAME_DRIVE) || // directory substitution on same drive
      (errorCode == ERROR_IS_SUBST_PATH) || // path is being used as substitute
      (errorCode == ERROR_IS_JOIN_PATH) || // not enough resources
      (errorCode == ERROR_PATH_BUSY) || // specified path cannot be used at this time
      (errorCode == ERROR_DIR_NOT_EMPTY) || // directory is not empty
      (errorCode == ERROR_IS_SUBST_TARGET) || // cannot substitute to another substitute
      (errorCode == ERROR_ALREADY_EXISTS) || // file or directory already exists
      (errorCode == ERROR_FILE_CHECKED_OUT) || // another user is locking the file
      (errorCode == ERROR_CHECKOUT_REQUIRED) || // file must be checked out for writing
      (errorCode == ERROR_BAD_FILE_TYPE) || // file type not allowed
      (errorCode == ERROR_FILE_TOO_LARGE) || // file size limit exceeded
      (errorCode == ERROR_VIRUS_INFECTED) || // file contains a virus
      (errorCode == ERROR_VIRUS_DELETED) || // file deleted because it contained a virus
      (errorCode == ERROR_DIRECTORY) || // invalid directory name
      (errorCode == ERROR_DISK_TOO_FRAGMENTED) || // volume is too fragmented
      (errorCode == ERROR_DELETE_PENDING) || // file is scheduled for deletion
      (errorCode == ERROR_DATA_CHECKSUM_ERROR) || // checksum error
      (errorCode == ERROR_DEVICE_UNREACHABLE) || // device could not be reached
      (errorCode == ERROR_DEVICE_NO_RESOURCES) || // device has no resources available
      (errorCode == ERROR_BAD_DEVICE_PATH) || // device path is invalid
      (errorCode == ERROR_COMPRESSED_FILE_NOT_SUPPORTED) || // not supported on compressed file
      (errorCode == ERROR_FILE_CORRUPT) || // File is damaged
      (errorCode == ERROR_DISK_CORRUPT) || // Drive is damaged
      (errorCode == ERROR_NOT_ENOUGH_QUOTA) // Write too large for process working set
    );
    if(isFileAccessError) {
      throw Errors::FileAccessError(
        std::error_code(errorCode, std::system_category()), combinedErrorMessage
      );
    } else {
      throw std::system_error(
        std::error_code(errorCode, std::system_category()), combinedErrorMessage
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForHResult(
    const std::string &errorMessage, HRESULT resultHandle
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(resultHandle));

    throw std::system_error(
      std::error_code(resultHandle, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
