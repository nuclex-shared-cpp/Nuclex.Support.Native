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

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Offers generic methods for dealing with the Posix API</summary>
  class PosixApi {

    /// <summary>Returns the error message for the specified error number</summary>
    /// <param name="errorNumber">
    ///   Error number for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error number</param>
    public: static std::string GetErrorMessage(int errorNumber);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorNumber">Value that 'errno' had at the time of failure</param>
    public: [[noreturn]] static void ThrowExceptionForSystemError(
      const std::string &errorMessage, int errorNumber
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXAPI_H
