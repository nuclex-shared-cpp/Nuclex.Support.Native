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

#include "Nuclex/Support/Text/CommandLine.h"

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  CommandLine::CommandLine() {}

  // ------------------------------------------------------------------------------------------- //

  CommandLine::~CommandLine() {}

  // ------------------------------------------------------------------------------------------- //

  CommandLine CommandLine::Parse(const std::string &parameterString) {
    (void)parameterString;
    return CommandLine();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
