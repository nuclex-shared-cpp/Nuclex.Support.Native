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

#include <celero/Celero.h>

#include "./../../Source/Text/NumberFormatter.h"

#include <algorithm> // for std::copy_n()
#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <string> // for std::string
#include <type_traits> // for std::is_signed
#include <cmath> // for std::abs()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Formats a number into a character string in a naive, slow way</summary>
  /// <typeparam name="TInteger">Integer type that will be converted</typeparam>
  /// <param name="integer">Integer that will be formatted into a string</param>
  /// <param name="buffer">Integer that will be formatted into a string</param>
  /// <returns>A pointer one past the last character written to the buffer</returns>
  template<typename TInteger>
  char *formatNumberNaive(char *buffer, TInteger integer) {
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
    std::size_t length = temp + sizeof(temp) - end;
    std::copy_n(end, length, buffer);
    return buffer + length;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937 randomNumberGenerator32;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator64;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint64_t> randomNumberDistribution64;

  /// <summary>Distribution for floats in a normalized -1.0 .. +1.0 range</summary>
  std::uniform_real_distribution<float> smallRandomNumberDistributionFloat(-1.0f, +1.0f);
  /// <summary>Distribution for floats using half of their entire numeric range</summary>
  std::uniform_real_distribution<float> largeRandomNumberDistributionFloat(
    std::numeric_limits<float>::lowest() / 2.1f, std::numeric_limits<float>::max() / 2.1f
  );

  /// <summary>Distribution for doubles in a normalized -1.0 .. +1.0 range</summary>
  std::uniform_real_distribution<double> smallRandomNumberDistributionDouble(-1.0f, +1.0f);
  /// <summary>Distribution for doubles using half of their entire numeric range</summary>
  std::uniform_real_distribution<double> largeRandomNumberDistributionDouble(
    std::numeric_limits<double>::lowest() / 2.1, std::numeric_limits<double>::max() / 2.1
  );

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Integer32Itoa, NaiveDivideBy10, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      formatNumberNaive(
        number,
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Integer64Itoa, NaiveDivideBy10, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      formatNumberNaive(
        number,
        static_cast<std::uint64_t>(randomNumberDistribution64(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Float32Ftoa_x2, CxxToString, 1000, 0) {
    celero::DoNotOptimizeAway(
      std::to_string(
        static_cast<float>(smallRandomNumberDistributionFloat(randomNumberGenerator64))
      )
    );
    celero::DoNotOptimizeAway(
      std::to_string(
        static_cast<float>(largeRandomNumberDistributionFloat(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Float64Ftoa_x2, CxxToString, 1000, 0) {
    celero::DoNotOptimizeAway(
      std::to_string(
        static_cast<double>(smallRandomNumberDistributionDouble(randomNumberGenerator64))
      )
    );
    celero::DoNotOptimizeAway(
      std::to_string(
        static_cast<double>(largeRandomNumberDistributionDouble(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, NumberFormatter, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      FormatInteger(
        number,
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer64Itoa, NumberFormatter, 1000, 0) {
    char number[40];
    celero::DoNotOptimizeAway(
      FormatInteger(
        number,
        static_cast<std::uint64_t>(randomNumberDistribution64(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Float32Ftoa_x2, NumberFormatter, 1000, 0) {
    char number[48];

    celero::DoNotOptimizeAway(
      FormatFloat(
        number,
        static_cast<float>(smallRandomNumberDistributionFloat(randomNumberGenerator64))
      )
    );
    celero::DoNotOptimizeAway(
      FormatFloat(
        number,
        static_cast<float>(largeRandomNumberDistributionFloat(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Float64Ftoa_x2, NumberFormatter, 1000, 0) {
    char number[325];

    celero::DoNotOptimizeAway(
      FormatFloat(
        number,
        static_cast<double>(smallRandomNumberDistributionDouble(randomNumberGenerator64))
      )
    );
    celero::DoNotOptimizeAway(
      FormatFloat(
        number,
        static_cast<double>(largeRandomNumberDistributionDouble(randomNumberGenerator64))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
