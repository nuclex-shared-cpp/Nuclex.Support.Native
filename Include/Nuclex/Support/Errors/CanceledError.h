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

#ifndef NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H
#define NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H

#include "Nuclex/Support/Config.h"

#include <future> // for std::future_error

namespace Nuclex { namespace Support { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a task or process has been canceled</summary>
  /// <remarks>
  ///   This is often used together with std::future as the exception that's assigned to
  ///   the std::future when its normal result is no longer going to arrive (for example,
  ///   because the thread performing the work is shutting down).
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE CanceledError : public std::future_error {

#if 0
    /// <summary>Initializes a cancellation-indicating error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CanceledError(const std::u8string &message);
#endif

    /// <summary>Initializes a cancellation-indicating error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CanceledError(const std::string &message) :
      std::future_error(std::future_errc::broken_promise),
      message(message) {}

    /// <summary>Initializes a cancellation-indicating error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CanceledError(const char *message) :
      std::future_error(std::future_errc::broken_promise),
      message(message) {}

    /// <summary>Retrieves an error message describing the cancellation reason</summary>
    /// <returns>A message describing te reason for the cancellation</returns>>
    public: NUCLEX_SUPPORT_API virtual const char *what() const noexcept override {
      if(this->message.empty()) {
        return std::future_error::what();
      } else {
        return this->message.c_str();
      }
    }

    /// <summary>Error message describing the reason for the cancellation</summary>
    private: std::string message;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H
