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
#include "./../../Source/Text/NumberFormatter.h"

#include <gtest/gtest.h>

#include <random>
#include <cmath>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Since we can't check all integers within a reasonable time, this is the number
  ///   of random checks we'll do to compare our integer formatter with std::to_string()
  /// </summary>
  const constexpr std::size_t SampleCount = 1'000;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, ThirtyTwoBitUnsignedIntegersAreFormattedCorrectly) {
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::uint32_t number = static_cast<std::uint32_t>(
        randomNumberDistribution32(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, ThirtyTwoBitSignedIntegersAreFormattedCorrectly) {
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<std::int32_t> randomNumberDistribution32(
      std::numeric_limits<std::int32_t>::min(),
      std::numeric_limits<std::int32_t>::max()
    );

    for(std::size_t index = 8; index < 13; ++index) {
      std::string expected = std::to_string(index);

      char buffer[40];
      char *end = FormatInteger(buffer, static_cast<std::uint32_t>(index));
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::int32_t number = static_cast<std::int32_t>(
        randomNumberDistribution32(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SixtyFourBitUnsignedIntegersAreFormattedCorrectly) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_int_distribution<std::uint64_t> randomNumberDistribution64;

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::uint64_t number = static_cast<std::uint64_t>(
        randomNumberDistribution64(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SixtyFourBitSignedIntegersAreFormattedCorrectly) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_int_distribution<std::int64_t> randomNumberDistribution64(
      std::numeric_limits<std::int64_t>::min(),
      std::numeric_limits<std::int64_t>::max()
    );

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::int64_t number = static_cast<std::int64_t>(
        randomNumberDistribution64(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LowestThirtyTwoBitSignedIntegersIsFormatted) {
    std::int32_t lowestValue = std::numeric_limits<std::int32_t>::min();

    std::string expected = std::to_string(lowestValue);

    char buffer[40];
    char *end = FormatInteger(buffer, lowestValue);
    std::string actual(buffer, end);

    EXPECT_EQ(expected, actual);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LowestSixtyFourBitSignedIntegersIsFormatted) {
    std::int64_t lowestValue = std::numeric_limits<std::int64_t>::min();

    std::string expected = std::to_string(lowestValue);

    char buffer[40];
    char *end = FormatInteger(buffer, lowestValue);
    std::string actual(buffer, end);

    EXPECT_EQ(expected, actual);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, FloatingPointValuesCanBePrinted) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_real_distribution<float> randomNumberDistribution;

    for(std::size_t index = 0; index < SampleCount; ++index) {
      float number = static_cast<float>(randomNumberDistribution(randomNumberGenerator));

      std::string expected = std::to_string(number);

      char buffer[48];
      char *end = FormatFloat(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
