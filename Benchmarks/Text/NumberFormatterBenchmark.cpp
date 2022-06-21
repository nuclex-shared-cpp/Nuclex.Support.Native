#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#include <celero/Celero.h>

#include "./../../Source/Text/Jeaiii/int_to_chars_jeaiii.h"
#include "./../../Source/Text/NumberFormatter.h"

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

  BASELINE(Integer32Itoa, JeaiiiOriginal, 30, 100000) {
    char number[40];
    celero::DoNotOptimizeAway(
      int_to_chars_jeaiii(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  char *itoa_better_y(std::uint32_t n, char *buffer);

  BENCHMARK(Integer32Itoa, JeaiiiImproved, 30, 100000) {
    char number[40];
    celero::DoNotOptimizeAway(
      itoa_better_y(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, JeaiiiDemacroed, 30, 100000) {
    char number[40];
    celero::DoNotOptimizeAway(
      FormatInteger(
        static_cast<std::uint32_t>(randomNumberDistribution(randomNumberGenerator)),
        number
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
