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

#include "Nuclex/Support/Text/ParserHelper.h"

#include <gtest/gtest.h>

#include <random>
#include <cmath>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;
  std::uniform_int_distribution<std::uint64_t> randomNumberDistribution64;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *formatInteger(std::uint32_t integerToFormat, char *buffer);
  char *formatInteger(std::uint64_t integerToFormat, char *buffer);

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, IntegersAreFormattedCorrectly32) {
    for(std::size_t index = 0; index < 500000; ++index) {
      std::uint32_t number = static_cast<std::uint32_t>(
        randomNumberDistribution32(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = formatInteger(number, buffer);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, IntegersAreFormattedCorrectly64) {
    for(std::size_t index = 0; index < 500000; ++index) {
      std::uint32_t number = static_cast<std::uint64_t>(
        randomNumberDistribution64(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = formatInteger(number, buffer);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

