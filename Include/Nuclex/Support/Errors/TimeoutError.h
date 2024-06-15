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

#ifndef NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H
#define NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H

#include "Nuclex/Support/Config.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Support { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a time-limited action was not completed within the alloted time
  /// </summary>
  class NUCLEX_SUPPORT_TYPE TimeoutError : public std::runtime_error {

    /// <summary>Initializes a new wait timeout error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit TimeoutError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new wait timeout error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit TimeoutError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H
