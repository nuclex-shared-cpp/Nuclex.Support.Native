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

#include "Nuclex/Support/Text/Logger.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Implementation of a logger that does not do anything</summary>
  class NullLogger : public Nuclex::Support::Text::Logger {

    /// <summary>Frees all resources owned by the logger</summary>
    public: virtual ~NullLogger() = default;

    /// <summary>Whether the logger is actually doing anything with the log messages</summary>
    /// <returns>True if the log messages are processed in any way, false otherwise</returns>
    /// <remarks>
    ///   Forming the log message strings may be non-trivial and cause memory allocations, too,
    ///   so by checking this method just once, you can skip all logging if they would be
    ///   discarded anyway.
    /// </remarks>
    public: virtual bool IsLogging() const override { return false; }

  };

  // ------------------------------------------------------------------------------------------- //

  NullLogger SharedNullLogger;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  Logger &Logger::Null = SharedNullLogger;

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
