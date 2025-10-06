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

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/StringHelper.h"

#include <celero/Celero.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simple substring removal method using standard C++ primitives</summary>
  /// <param name="master">String from which substrings will be removed</param>
  /// <param name="substringToRemove">Substring of which all occurrences will be removed</param>
  void removeAllOccurrenceNaive(std::u8string &master, const std::u8string &substringToRemove) {
    for(;;) {
      std::u8string::size_type index = master.find(substringToRemove);
      if(index == std::u8string::npos) {
        return; // All occurrences removed
      }

      master.erase(index, substringToRemove.length());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tests the removal of substrings via standard C++ primitives</summary>
  /// <param name="master">Master string with which the tests will be performed</param>
  /// <param name="substringToRemove">Substring to test the removal with</param>
  /// <returns>
  ///   A value dependent on the operation that can be used to prevent the optimizer
  ///   from optimizing the entire method call away
  /// </returns>
  bool testNaiveRemoval(const std::u8string &master, const std::u8string &substringToRemove) {
    std::u8string masterCopy = master;
    removeAllOccurrenceNaive(masterCopy, substringToRemove);
    return masterCopy.empty();
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tests the removal of substrings via the custom StringHelper method</summary>
  /// <param name="master">Master string with which the tests will be performed</param>
  /// <param name="substringToRemove">Substring to test the removal with</param>
  /// <returns>
  ///   A value dependent on the operation that can be used to prevent the optimizer
  ///   from optimizing the entire method call away
  /// </returns>
  bool testStringHelperRemoval(
    const std::u8string &master, const std::u8string &substringToRemove
  ) {
    std::u8string masterCopy = master;
    Nuclex::Support::Text::StringHelper::EraseSubstrings(masterCopy, substringToRemove);
    return masterCopy.empty();
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BASELINE(SubstringRemoval, ViaCxxMethods, 1000, 0) {
    std::u8string master(
      u8"This <mooh> is a longer string <mooh> which may or may not <mooh> have "
      u8"been spoken <mooh> by a trained bovine.",
      110
    );

    celero::DoNotOptimizeAway(
      testNaiveRemoval(master, u8"<mooh> ")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(SubstringRemoval, ViaStringHelper, 1000, 0) {
    std::u8string master(
      u8"This <mooh> is a longer string <mooh> which may or may not <mooh> have "
      u8"been spoken <mooh> by a trained bovine.",
      110
    );

    celero::DoNotOptimizeAway(
      testStringHelperRemoval(master, u8"<mooh> ")
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
