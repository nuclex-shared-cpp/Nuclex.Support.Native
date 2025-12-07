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

#include "Nuclex/Support/Endian.h"

#include <gtest/gtest.h>

#include <random> // lots, for testing with random numbers

namespace Nuclex::Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, EndianFlippingEightBitIntegerDoesNothing) {
    std::uint8_t before(123);
    EXPECT_EQ(before, Endian::Flip(before));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, SixteenBitIntegerCanBeEndianFlipped) {
    std::uint16_t before(32109);
    EXPECT_NE(before, Endian::Flip(before));

    std::uint16_t flipped = Endian::Flip(before);
    EXPECT_EQ(before, Endian::Flip(flipped));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, ThirtyTwoBitIntegerCanBeEndianFlipped) {
    std::uint32_t before(2109876543);
    EXPECT_NE(before, Endian::Flip(before));

    std::uint32_t flipped = Endian::Flip(before);
    EXPECT_EQ(before, Endian::Flip(flipped));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, SixtyFourBitIntegerCanBeEndianFlipped) {
    std::uint64_t before(9012345678901234567ULL);
    EXPECT_NE(before, Endian::Flip(before));

    std::uint64_t flipped = Endian::Flip(before);
    EXPECT_EQ(before, Endian::Flip(flipped));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, ProvidesHostFromNetworkEndianHelper) {
    std::uint8_t eightBitResult = HostFromNetworkEndian::Flip(std::uint8_t(0));
    (void)eightBitResult;

    std::uint16_t sixteenBitResult = HostFromNetworkEndian::Flip(std::uint16_t(0));
    (void)sixteenBitResult;

    std::uint32_t thirtyTwoBitResult = HostFromNetworkEndian::Flip(std::uint32_t(0));
    (void)thirtyTwoBitResult;

    std::uint64_t sixtyFourBitResult = HostFromNetworkEndian::Flip(std::uint64_t(0));
    (void)sixtyFourBitResult;
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianTest, ProvidesNetworkFromHostEndianHelper) {
    std::uint8_t eightBitResult = NetworkFromHostEndian::Flip(std::uint8_t(0));
    (void)eightBitResult;

    std::uint16_t sixteenBitResult = NetworkFromHostEndian::Flip(std::uint16_t(0));
    (void)sixteenBitResult;

    std::uint32_t thirtyTwoBitResult = NetworkFromHostEndian::Flip(std::uint32_t(0));
    (void)thirtyTwoBitResult;

    std::uint64_t sixtyFourBitResult = NetworkFromHostEndian::Flip(std::uint64_t(0));
    (void)sixtyFourBitResult;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support
