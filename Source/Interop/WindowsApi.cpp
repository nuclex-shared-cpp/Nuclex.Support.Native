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

#include <comdef.h> // for _com_error

#include "Nuclex/Support/Errors/FileAccessError.h" // for FileAccessError

#include "Nuclex/Support/Text/StringConverter.h" // to convert UTF-16 wholesome
#include "Nuclex/Support/Text/ParserHelper.h" // to skip trailing whitespace
#include "Nuclex/Support/Text/LexicalAppend.h" // to append error codes to strings
#include "Nuclex/Support/Text/UnicodeHelper.h" // for UTF-16 <-> UTF-8 conversion

#include <vector> // for std::vector
#include <system_error> // for std::system_error

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

  /// <summary>Wrapper around FormatMessage() that extract the error message</summary>
  /// <param name="errorCode">
  ///   DWORD error code or standard HRESULT to query the error message for
  /// </param>
  /// <param name="fallbackMessage">
  ///   Message to fall back to if FormatMessage() does not know the error code or HRESULT
  ///   (should end with a space because the numeric error code will be appended to it)
  /// </param>
  /// <returns>A printable error message always encoded in UTF-8</returns>
  std::u8string callFormatMessage(DWORD errorCode, const std::u8string &fallbackMessage) {
    using Nuclex::Support::Text::UnicodeHelper;
    using Nuclex::Support::Text::ParserHelper;

    // Use FormatMessage() to ask Windows for a human-readable error message.
    // First, we'll ask for an English message regardless of the system language in
    // order to provide an understandable (and internet-searchable) message if possible.
    LPWSTR wideErrorMessageBuffer;
    DWORD wideErrorMessageLength = ::FormatMessageW(
      (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS
      ),
      nullptr, // message string, ignored with passed flags
      errorCode,
      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Try for an English message first
      reinterpret_cast<LPWSTR>(&wideErrorMessageBuffer), // MS wants us to cast off pointer levels!
      0,
      nullptr
    );
    if(wideErrorMessageLength == 0) {
      // MSDN states that "Last-Error" will be set to ERROR_RESOURCE_LANG_NOT_FOUND,
      // but that doesn't really happen, so we recheck on *any* FormatMessage() failure
      wideErrorMessageLength = ::FormatMessageW(
        (
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS
        ),
        nullptr, // message string, ignored with passed flags
        errorCode,
        0, // Let FormatMessage search: neutral, thread locale, user locale and system locale
        reinterpret_cast<LPWSTR>(&wideErrorMessageBuffer), // MS wants us to cast off pointer levels!
        0,
        nullptr
      );
      if(wideErrorMessageLength == 0) [[unlikely]] {
        std::u8string errorMessage(fallbackMessage);
        Nuclex::Support::Text::lexical_append(errorMessage, static_cast<std::uint32_t>(errorCode));
        return errorMessage;
      }
    }

    // The error message itself will have been allocated on the process heap,
    // so we need to make sure to always delete it to avoid memory leaks
    LocalAllocScope errorMessageScope(wideErrorMessageBuffer);

    // We don't want UTF-16 anywhere - at all. So convert this mess to UTF-8.
    std::u8string errorMessage;
    {
      errorMessage.resize(wideErrorMessageLength);

      // We're on Windos, so wchar_t is char16_t (on Linux it would be char32_t)
      const char16_t *current = reinterpret_cast<const char16_t *>(wideErrorMessageBuffer);
      const char16_t *end = current + wideErrorMessageLength;

      char8_t *write = errorMessage.data();
      while(current < end) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
        if(codePoint == char32_t(-1)) {
          break;
        }
        UnicodeHelper::WriteCodePoint(write, codePoint);
      }

      errorMessage.resize(write - errorMessage.data());
    }

    // Microsoft likes to end their error messages with various spaces and newlines,
    // cut these off so we have a single-line error message
    std::u8string::size_type length = errorMessage.length();
    while(length > 0) {
      if(!ParserHelper::IsWhitespace(static_cast<char>(errorMessage[length - 1]))) {
        break;
      }
      --length;
    }

    // Since the error message is trimmed of any trailing whitespace (including \r and \n),
    // it may actually end up empty. As unlikely as that is, to be entirely on the safe side,
    // we check this and use the fallback message should it really happen.
    if(length == 0) [[unlikely]] {
      errorMessage.assign(fallbackMessage);
      Nuclex::Support::Text::lexical_append(
        errorMessage, static_cast<std::uint32_t>(errorCode)
      );
    } else {
      errorMessage.resize(length);
    }

    return errorMessage;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  std::u8string WindowsApi::GetErrorMessage(int errorNumber) {
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
      std::u8string errorMessage(u8"Error ");
      Text::lexical_append(errorMessage, errorNumber);
      errorMessage.append(u8" (and error message lookup failed)");
      return errorMessage;

    } // for(;;)
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string WindowsApi::GetErrorMessage(DWORD errorCode) {
    const static std::u8string fallbackMessage(u8"Windows API error ");
    return callFormatMessage(errorCode, fallbackMessage);
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string WindowsApi::GetErrorMessage(HRESULT resultHandle) {

    // All HRESULTs in the in the "interface" facility (meaning all the common errors
    // from 0x80040000 to 0x804FFFF) map to the classic 16-bit Windows error codes
    // (called "WCode" in the _com_error class).
    //
    // This article claims 0x80040200 to 0x8004FFFF:
    // https://learn.microsoft.com/en-us/cpp/cpp/com-error-wcode
    //
    // But that would cut off the ever popular E_FAIL, E_NOTIMPL, E_POINTER.
    //

    // Then we've got E_INVALIDARG, E_ACCESSDENIED and such in the "win32" facility
    // where we find another region of 16-bit error codes, called "system error codes"
    // in recent Microsoft documentation.
    //
    // See HRESULT_FROM_WIN32() here:
    // https://learn.microsoft.com/en-us/windows/win32/api/winerror/
    //

    // Microsoft code often shovels these and more into FormatMessage(), unmodified,
    // and hopes that it can figure out an error message (perhaps relying on it to
    // fail if the error code doesn't exist). 
    //
    // So that's what we'll do here as well:
    //
    const static std::u8string fallbackMessage(u8"Windows COM error ");
    return callFormatMessage(static_cast<DWORD>(resultHandle), fallbackMessage);

  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForSystemError(
    const std::u8string &errorMessage, DWORD errorCode
  ) {
    std::u8string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(errorCode));

    throw std::system_error(
      std::error_code(errorCode, std::system_category()),
      reinterpret_cast<const char *>(combinedErrorMessage.c_str())
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForFileSystemError(
    const std::u8string &errorMessage, DWORD errorCode
  ) {
    std::u8string combinedErrorMessage(errorMessage);
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
      (errorCode == ERROR_LOCK_VIOLATION) || // another process has locked the file
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
        std::error_code(errorCode, std::system_category()),
        reinterpret_cast<const char *>(combinedErrorMessage.c_str())
      );
    } else {
      throw std::system_error(
        std::error_code(errorCode, std::system_category()),
        reinterpret_cast<const char *>(combinedErrorMessage.c_str())
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForHResult(
    const std::u8string &errorMessage, HRESULT resultHandle
  ) {
    std::u8string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(resultHandle));

    throw std::system_error(
      std::error_code(resultHandle, std::system_category()),
      reinterpret_cast<const char *>(combinedErrorMessage.c_str())
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
