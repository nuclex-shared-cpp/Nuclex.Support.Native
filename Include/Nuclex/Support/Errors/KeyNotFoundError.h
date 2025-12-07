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

#ifndef NUCLEX_SUPPORT_ERRORS_KEYNOTFOUNDERROR_H
#define NUCLEX_SUPPORT_ERRORS_KEYNOTFOUNDERROR_H

#include "Nuclex/Support/Config.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex::Support::Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that the key did not exist when an item is accessed by key</summary>
  class NUCLEX_SUPPORT_TYPE KeyNotFoundError : public std::out_of_range {

    /// <summary>Initializes a new keu not found error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit KeyNotFoundError(const std::string &message) :
      std::out_of_range(message) {}

    /// <summary>Initializes a new key not found error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit KeyNotFoundError(const char *message) :
      std::out_of_range(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_KEYNOTFOUNDERROR_H
