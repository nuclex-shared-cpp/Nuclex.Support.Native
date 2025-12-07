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

#ifndef NUCLEX_SUPPORT_PLATFORM_POSIXAPI_H
#define NUCLEX_SUPPORT_PLATFORM_POSIXAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include <string> // for std::u8string

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Controls the error handling of wrapper functions around C APIs</summary>
  enum class ErrorPolicy {

    /// <summary>Any non-successful outcome will result in an exception</summary>
    /// <remarks>
    ///   Some very specific errors (i.e. starting a file enumeration on Windows will return
    ///   <code>ERROR_FILE_NOT_FOUND</code> if a directory is empty, which is obviously
    ///   a regular outcome, or Linux threading functions that return EAGAIN or EBUSY) will
    ///   be returned as normalized boolean or <code>std::optional&lt;&gt;</code> results.
    /// </remarks>
    Throw = -1,

    /// <summary>Non-successful outcomes will trigger an assertion in debug mode</summary>
    /// <remarks>
    ///   This is intended for RAII cleanup calls to avoid throwing in the destructor (at
    ///   the price of silently leaking a resource in release mode, though typical close and
    ///   release functions are designed to never fail under normal circumstances).
    /// </remarks>
    Assert = 0

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Offers generic methods for dealing with the Posix API</summary>
  class PosixApi {

    /// <summary>Returns the error message for the specified error number</summary>
    /// <param name="errorNumber">
    ///   Error number for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error number</param>
    public: static std::u8string GetErrorMessage(int errorNumber);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorNumber">Value that 'errno' had at the time of failure</param>
    public: [[noreturn]] static void ThrowExceptionForSystemError(
      const std::u8string &errorMessage, int errorNumber
    );

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorNumber">Value that 'errno' had at the time of failure</param>
    /// <remarks>
    ///   This variant is intended to be used when encountered error results from
    ///   calls that open, read or write files. It will transform certain permission
    ///   denied errors into FileAccessError exceptions.
    /// </remarks>
    public: [[noreturn]] static void ThrowExceptionForFileAccessError(
      const std::u8string &errorMessage, int errorNumber
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXAPI_H
