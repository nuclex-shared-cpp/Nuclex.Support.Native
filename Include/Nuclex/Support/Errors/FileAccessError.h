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

#ifndef NUCLEX_SUPPORT_ERRORS_FILEACCESSERROR_H
#define NUCLEX_SUPPORT_ERRORS_FILEACCESSERROR_H

#include "Nuclex/Support/Config.h"

#include <system_error> // for std::system_error

namespace Nuclex::Support::Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a file was not found or could not be opened</summary>
  /// <remarks>
  ///   <para>
  ///     This error will be thrown if anything went wrong opening or accessing a file
  ///     in Nuclex.Support.Native or other parts of the framework.
  ///   </para>
  ///   <para>
  ///     If you get this error while working with Nuclex.Pixels or Nuclex.Audio,
  ///     for example, it means that your load or save operation has failed not due to
  ///     a problem with the library or codec, but in the underlying stream - a file may
  ///     be unreadable, you may not be allowed to access it or your custom virtual file
  ///     implementation failed to fetch or transmit data.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE FileAccessError : public std::system_error {

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit FileAccessError(
      std::error_code errorCode, const std::string &message
    ) : std::system_error(errorCode, message) {}

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit FileAccessError(
      std::error_code errorCode, const char *message
    ) : std::system_error(errorCode, message) {}

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_FILEACCESSERROR_H
