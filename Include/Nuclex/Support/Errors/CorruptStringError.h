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

#ifndef NUCLEX_SUPPORT_ERRORS_CORRUPTSTRINGERROR_H
#define NUCLEX_SUPPORT_ERRORS_CORRUPTSTRINGERROR_H

#include "Nuclex/Support/Config.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Support { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a unicode string was malformed / contained invalid characters
  /// </summary>
  class NUCLEX_SUPPORT_TYPE CorruptStringError : public std::runtime_error {

    /// <summary>Initializes a new corrupt string error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CorruptStringError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new corrupt string error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CorruptStringError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_CORRUPTSTRINGERROR_H
