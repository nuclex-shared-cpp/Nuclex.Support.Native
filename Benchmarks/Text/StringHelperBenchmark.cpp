#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

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
  void removeAllOccurrenceNaive(std::string &master, const std::string &substringToRemove) {
    for(;;) {
      std::string::size_type index = master.find(substringToRemove);
      if(index == std::string::npos) {
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
  bool testNaiveRemoval(const std::string &master, const std::string &substringToRemove) {
    std::string masterCopy = master;
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
  bool testStringHelperRemoval(const std::string &master, const std::string &substringToRemove) {
    std::string masterCopy = master;
    Nuclex::Support::Text::StringHelper::EraseSubstrings(masterCopy, substringToRemove);
    return masterCopy.empty();
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BASELINE(SubstringRemoval, ViaCxxMethods, 1000, 0) {
    std::string master(
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
    std::string master(
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
