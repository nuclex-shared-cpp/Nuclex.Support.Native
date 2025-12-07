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

#ifndef NUCLEX_SUPPORT_PLATFORM_ERRORPOLICY_H
#define NUCLEX_SUPPORT_PLATFORM_ERRORPOLICY_H

#include "Nuclex/Support/Config.h"

namespace Nuclex::Support::Interop {

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

} // namespace Nuclex::Support::Interop

#endif // NUCLEX_SUPPORT_PLATFORM_ERRORPOLICY_H
