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

#ifndef NUCLEX_SUPPORT_INTEROP_WINDOWSAPI_H
#define NUCLEX_SUPPORT_INTEROP_WINDOWSAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

// The Windows headers tend to include a ton of crap and pollute the global namespace
// like nothing else. These macros cut down on that a bit.
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

// These symbols are redefined globally when you include the Windows header. Needless
// to say that we don't want that, because it would result in those names chaotically
// changing between implementation and public header - and even in the application using
// this library depending on whether the user included Windows.h somewhere.
#undef CreateFile
#undef DeleteFile
#undef MoveFile
#undef CreateDirectory
#undef RemoveDirectory
#undef GetFileAttributes
#undef GetFileAttributesEx
#undef FindFirstFile
#undef FindNextFile
#undef GetTempPath
#undef GetModuleFileName
#undef GetFullPathName
#undef GetWindowsDirectory
#undef GetSystemDirectory

#include <string> // for std::u8string

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Offers generic methods for dealing with the Windows API</summary>
  class WindowsApi {

    /// <summary>Returns the error message for the specified error number</summary>
    /// <param name="errorNumber">
    ///   Error number for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error number</param>
    /// <remarks>
    ///   Some Posix methods can also be found in the Windows API, usually with
    ///   non-standard underscore prefixes. For these methods, Microsoft's reimplementation
    ///   of strerror(), named _wcserror_s(), needs to be used with the error number
    ///   found in the 'errno' variable (like on Posix systems). This method handles
    ///   calling _wcserror_s() to obtain a meaningful error message for 'errno'.
    /// </remarks>
    public: static std::u8string GetErrorMessage(int errorNumber);

    /// <summary>Returns the error message for the specified error code</summary>
    /// <param name="errorCode">
    ///   Error code for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error code</param>
    /// <remarks>
    ///   Standard Windows API methods that only exist on Microsoft systems usually
    ///   signal error/success with their return code. The actual error type can be
    ///   looked up by calling GetLastError(). This method fetches a meaningful error
    ///   message for the error code returned by GetLastError().
    /// </remarks>
    public: static std::u8string GetErrorMessage(DWORD errorCode);

    /// <summary>Returns the error message for the specified HRESULT</summary>
    /// <param name="resultHandle">
    ///   HRESULT for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified HRESULT</param>
    /// <remarks>
    ///   COM (a cross-language ABI that defines vtable layout, calling convention,
    ///   error handling etc.) uses HRESULTs for all method returns. A HRESULT is
    ///   a combination of flags, the most significant bit indicates error/success
    ///   (so all negative HRESULTS are error codes). This method fetches a meaningful
    ///   error message for the HRESULT returned by a COM method.
    /// </remarks>
    public: static std::u8string GetErrorMessage(HRESULT resultHandle);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorCode">
    ///   Value that GetLastError() returned at the time of failure
    /// </param>
    public: [[noreturn]] static void ThrowExceptionForSystemError(
      const std::u8string &errorMessage, DWORD errorCode
    );

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorCode">
    ///   Value that GetLastError() returned at the time of failure
    /// </param>
    /// <remarks>
    ///   This variant is intended to be used with error codes returned by file sytem
    ///   functions. It will throw a FileAccessError exception is the error is related
    ///   to opening, reading or writing to files.
    /// </remarks>
    public: [[noreturn]] static void ThrowExceptionForFileSystemError(
      const std::u8string &errorMessage, DWORD errorCode
    );

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="resultHandle">HRESULT that was returned by the failed function</param>
    public: [[noreturn]] static void ThrowExceptionForHResult(
      const std::u8string &errorMessage, HRESULT resultHandle
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_INTEROP_WINDOWSAPI_H
