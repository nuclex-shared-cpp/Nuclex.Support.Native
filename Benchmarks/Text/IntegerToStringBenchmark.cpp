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
#include "Nuclex/Support/Text/LexicalCast.h"

#include <celero/Celero.h>

#include <algorithm> // for std::copy_n()
#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <string> // for std::string
#include <type_traits> // for std::is_signed
#include <cmath> // for std::abs()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Formats a number into an std::string in a naive, slow way</summary>
  /// <typeparam name="TInteger">Integer type that will be converted</typeparam>
  /// <param name="integer">Integer that will be formatted into a string</param>
  /// <returns>A string containing a textual representation  of the integer</returns>
  template<typename TInteger>
  std::string formatNumberNaive(TInteger integer) {
    char temp[40]; // max 128 bit integer length without terminating \0

    // If the integer may be negative, remember it and make it positive
    if constexpr(std::is_signed<TInteger>::value) {
      if(integer < 0) {
        temp[0] = u8'-';
        integer = std::abs(integer);
      }
    }

    // Build the integer backwards by successively dividing it by 10
    char *end = temp + sizeof(temp) - 1;
    while(integer >= 10) {
      *end = static_cast<char>(u8'0' + (integer % 10));
      integer /= 10;
      --end; // go backwards
    }
    *end = static_cast<char>(u8'0' + integer);

    // If the integer may be negative, and was negative, prepend a minus sign
    if constexpr(std::is_signed<TInteger>::value) {
      if(temp[0] == u8'-') {
        --end;
        *end = u8'-';
      }
    }

    // Package the generated character in an std::string
    return std::string(end, temp + sizeof(temp) - end);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Integer32ToString, CxxToString, 1000, 0) {
    celero::DoNotOptimizeAway(
      std::to_string(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32ToString, NaiveDivideBy10, 1000, 0) {
    celero::DoNotOptimizeAway(
      formatNumberNaive(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32ToString, NuclexLexicalCast, 1000, 0) {
    celero::DoNotOptimizeAway(
      Nuclex::Support::Text::lexical_cast<std::string>(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
