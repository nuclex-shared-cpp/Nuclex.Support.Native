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

#if defined(HAVE_JEAIII_ITOA)

#include "Nuclex/Support/BitTricks.h" // for BitTricks::GetLogBase10()

#include <celero/Celero.h>

#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <type_traits> // for std::integral_constant

// Uses a magic formula to turn a 32 bit number into a specific 64 bit number.
//
// I think the main thing this formula accomplishes is that the actual number sits at
// the upper end of a 32 bit integer. Thus, when you cast it to a 64 bit integer and
// multiply it by 100, you end up with the next two digits in the upper 32 bits of
// your 64 bit integer where they're easy to grab.
//
// Magnitude is 1 for 100, 2 for 1'000, 3 for 10'000 and so on
//
#define PREPARE_NUMBER_OF_MAGNITUDE(number, magnitude) \
  temp = ( \
    (std::uint64_t(1) << (32 + magnitude / 5 * magnitude * 53 / 16)) / \
    std::uint32_t(1e##magnitude) + 1 + magnitude/6 - magnitude/8 \
  ), \
  temp *= number, \
  temp >>= magnitude / 5 * magnitude * 53 / 16, \
  temp += magnitude / 6 * 4

// Brings the next two digits of the prepeared number into the upper 32 bits
// so they can be extracted by the WRITE_ONE_DIGIT and WRITE_TWO_DIGITS macros
#define READY_NEXT_TWO_DIGITS() \
  temp = std::uint64_t(100) * static_cast<std::uint32_t>(temp)

// Appends the next two highest digits in the prepared number to the char buffer
// Also adjusts the number such that the next two digits are ready for extraction.
#define WRITE_TWO_DIGITS(bufferPointer) \
  *reinterpret_cast<TwoChars *>(bufferPointer) = ( \
    *reinterpret_cast<const TwoChars *>(&Radix100[(temp >> 31) & 0xFE]) \
  )

// Appends the next highest digit in the prepared number to the char buffer
// Thus doesn't adjust the number because it is always used on the very last digit.
#define WRITE_ONE_DIGIT(bufferPointer) \
  *reinterpret_cast<char *>(bufferPointer) = ( \
    u8'0' + static_cast<char>(std::uint64_t(10) * std::uint32_t(temp) >> 32) \
  )

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Table of the numbers 00 .. 99 as a flat array</summary>
  /// <remarks>
  ///   Used for James Edward Anhalt III.'s integer formatting technique where two digits
  ///   are converted at once, among other tricks.
  /// </remarks>
  constexpr char Radix100[] = {
    u8'0', u8'0',   u8'0', u8'1',   u8'0', u8'2',   u8'0', u8'3',   u8'0', u8'4',
    u8'0', u8'5',   u8'0', u8'6',   u8'0', u8'7',   u8'0', u8'8',   u8'0', u8'9',
    u8'1', u8'0',   u8'1', u8'1',   u8'1', u8'2',   u8'1', u8'3',   u8'1', u8'4',
    u8'1', u8'5',   u8'1', u8'6',   u8'1', u8'7',   u8'1', u8'8',   u8'1', u8'9',
    u8'2', u8'0',   u8'2', u8'1',   u8'2', u8'2',   u8'2', u8'3',   u8'2', u8'4',
    u8'2', u8'5',   u8'2', u8'6',   u8'2', u8'7',   u8'2', u8'8',   u8'2', u8'9',
    u8'3', u8'0',   u8'3', u8'1',   u8'3', u8'2',   u8'3', u8'3',   u8'3', u8'4',
    u8'3', u8'5',   u8'3', u8'6',   u8'3', u8'7',   u8'3', u8'8',   u8'3', u8'9',
    u8'4', u8'0',   u8'4', u8'1',   u8'4', u8'2',   u8'4', u8'3',   u8'4', u8'4',
    u8'4', u8'5',   u8'4', u8'6',   u8'4', u8'7',   u8'4', u8'8',   u8'4', u8'9',
    u8'5', u8'0',   u8'5', u8'1',   u8'5', u8'2',   u8'5', u8'3',   u8'5', u8'4',
    u8'5', u8'5',   u8'5', u8'6',   u8'5', u8'7',   u8'5', u8'8',   u8'5', u8'9',
    u8'6', u8'0',   u8'6', u8'1',   u8'6', u8'2',   u8'6', u8'3',   u8'6', u8'4',
    u8'6', u8'5',   u8'6', u8'6',   u8'6', u8'7',   u8'6', u8'8',   u8'6', u8'9',
    u8'7', u8'0',   u8'7', u8'1',   u8'7', u8'2',   u8'7', u8'3',   u8'7', u8'4',
    u8'7', u8'5',   u8'7', u8'6',   u8'7', u8'7',   u8'7', u8'8',   u8'7', u8'9',
    u8'8', u8'0',   u8'8', u8'1',   u8'8', u8'2',   u8'8', u8'3',   u8'8', u8'4',
    u8'8', u8'5',   u8'8', u8'6',   u8'8', u8'7',   u8'8', u8'8',   u8'8', u8'9',
    u8'9', u8'0',   u8'9', u8'1',   u8'9', u8'2',   u8'9', u8'3',   u8'9', u8'4',
    u8'9', u8'5',   u8'9', u8'6',   u8'9', u8'7',   u8'9', u8'8',   u8'9', u8'9'
  };

  // ------------------------------------------------------------------------------------------- //

  struct TwoChars { char t, o; };

  // ------------------------------------------------------------------------------------------- //

  struct JeaiiiValues {
    std::uint32_t Factor;
    std::uint32_t Shift;
    std::uint32_t Bias;
  };

  // ------------------------------------------------------------------------------------------- //

  const JeaiiiValues magic[] = {
    {             0,  0, 0 }, // magnitude 1e-1 (invalid)
    {             0,  0, 0 }, // magnitude 1e0 (invalid) (4'294'967'297)
    {   429'496'730,  0, 0 }, // magnitude 1e1
    {    42'949'673,  0, 0 }, // magnitude 1e2
    {     4'294'968,  0, 0 }, // magnitude 1e3
    {       429'497,  0, 0 }, // magnitude 1e4
    { 2'814'749'768, 16, 0 }, // magnitude 1e5
    { 2'251'799'815, 19, 4 }, // magnitude 1e6
    { 3'602'879'703, 23, 4 }, // magnitude 1e7
    { 2'882'303'762, 26, 4 }, // magnitude 1d8
    { 2'305'843'010, 29, 4 }, // magnitude 1e9
    {             5, 66, 4 }  // magnitude 1e10
  };

  // ------------------------------------------------------------------------------------------- //

  const std::uint32_t factors[] = {
                0, // magnitude 1e-1 (invalid)
                0, // magnitude 1e0 (invalid) (4'294'967'297)
      429'496'730, // magnitude 1e1
       42'949'673, // magnitude 1e2
        4'294'968, // magnitude 1e3
          429'497, // magnitude 1e4
    2'814'749'768, // magnitude 1e5
    2'251'799'815, // magnitude 1e6
    3'602'879'703, // magnitude 1e7
    2'882'303'762, // magnitude 1d8
    2'305'843'010, // magnitude 1e9
                5  // magnitude 1e10
  };

  // ------------------------------------------------------------------------------------------- //

  const std::uint32_t shift[] = {
     0, // magnitude 1e-1 (invalid)
     0, // magnitude 1e0 (invalid)
     0, // magnitude 1e1
     0, // magnitude 1e2
     0, // magnitude 1e3
     0, // magnitude 1e4
    16, // magnitude 1e5
    19, // magnitude 1e6
    23, // magnitude 1e7
    26, // magnitude 1e8
    29, // magnitude 1e9
    66  // magnitude 1e10
  };

  // ------------------------------------------------------------------------------------------- //

  const std::uint32_t bias[] = {
    0, // magnitude 1e-1 (invalid)
    0, // magnitude 1e0 (invalid)
    0, // magnitude 1e1
    0, // magnitude 1e2
    0, // magnitude 1e3
    0, // magnitude 1e4
    0, // magnitude 1e5
    4, // magnitude 1e6
    4, // magnitude 1e7
    4, // magnitude 1e8
    4, // magnitude 1e9
    4  // magnitude 1e10
  };

  // ------------------------------------------------------------------------------------------- //

  char *jeaiiiLutItoa(char *buffer /* [10] */, std::uint32_t number) {
    int magnitude = Nuclex::Support::BitTricks::GetLogBase10(number);

    std::uint64_t temp = number;
    temp *= factors[magnitude];
    temp >>= shift[magnitude];
    temp += bias[magnitude];

    // If it's just one digit, skip the two-digit-pull loop and just
    // output that lone digit
    if(magnitude == 0) {
      WRITE_ONE_DIGIT(buffer);
      return buffer + 1;
    }

    // If there are at least two digits, turn them into text in pairs until
    // less than two are left.
    for(;;) {
      WRITE_TWO_DIGITS(buffer);
      if(magnitude < 2) { // Are less than 2 remaining?
        if(magnitude >= 1) { // is even 1 remaining?
          WRITE_ONE_DIGIT(buffer);
          return buffer + 3;
        } else {
          return buffer + 2;
        }
      }
      READY_NEXT_TWO_DIGITS();
      magnitude -= 2;
      buffer += 2;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *jeaiiiStructLutItoa(char *buffer /* [10] */, std::uint32_t number) {
    int magnitude = Nuclex::Support::BitTricks::GetLogBase10(number);

    std::uint64_t temp = number;
    {
      const JeaiiiValues &magicValues = magic[magnitude];
      temp *= magicValues.Factor;
      temp >>= magicValues.Shift;
      temp += magicValues.Bias;
    }

    // If it's just one digit, skip the two-digit-pull loop and just
    // output that lone digit
    if(magnitude == 0) {
      WRITE_ONE_DIGIT(buffer);
      return buffer + 1;
    }

    // If there are at least two digits, turn them into text in pairs until
    // less than two are left.
    for(;;) {
      WRITE_TWO_DIGITS(buffer);
      if(magnitude < 2) { // Are less than 2 remaining?
        if(magnitude >= 1) { // is even 1 remaining?
          WRITE_ONE_DIGIT(buffer);
          return buffer + 3;
        } else {
          return buffer + 2;
        }
      }
      READY_NEXT_TWO_DIGITS();
      magnitude -= 2;
      buffer += 2;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937 randomNumberGenerator32;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, JeaiiiLut, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      jeaiiiLutItoa(
        number,
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Integer32Itoa, JeaiiiStructLut, 1000, 0) {
    char number[40];

    celero::DoNotOptimizeAway(
      jeaiiiStructLutItoa(
        number,
        static_cast<std::uint32_t>(randomNumberDistribution32(randomNumberGenerator32))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#undef WRITE_TWO_DIGITS
#undef WRITE_ONE_DIGIT
#undef READY_NEXT_TWO_DIGITS
#undef PREPARE_NUMBER_OF_MAGNITUDE

#endif // defined(HAVE_JEAIII_ITOA)
