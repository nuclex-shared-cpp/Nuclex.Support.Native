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

#include "./NumberFormatter.h"
#include "./DragonBox-1.1.2/dragonbox.h" // for the float-to-decimal algorithm
#include "Nuclex/Support/BitTricks.h" // for BitTricks::GetLogBase10()

#include <iostream>

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
    *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[(temp >> 31) & 0xFE]) \
  )

// Appends the next highest digit in the prepared number to the char buffer
// Thus doesn't adjust the number because it is always used on the very last digit.
#define WRITE_ONE_DIGIT(bufferPointer) \
  *reinterpret_cast<char *>(bufferPointer) = ( \
    u8'0' + static_cast<char>(std::uint64_t(10) * std::uint32_t(temp) >> 32) \
  )

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Structure with the size of two chars</summary>
  /// <remarks>
  ///   This is only used to assign two characters at once. Benchmarks (in release mode on
  ///   AMD64 with -O3 on GCC 11) revealed that std::memcpy() is not inlined/intrinsic'd as
  ///   much as one would hope and that this method resulted in faster code.
  /// </remarks>
  struct TwoChars { char t, o; };

  struct JeaiiiValues {
    std::uint32_t Factor;
    std::uint32_t Shift;
    std::uint32_t Bias;
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

  char *formatIntegerSimple(char *buffer /* [10] */, std::uint32_t number, int magnitude) {
    std::uint64_t temp = number;
    temp *= factors[magnitude];
    temp >>= shift[magnitude];
    temp += bias[magnitude];

    if(magnitude == 0) {
      WRITE_ONE_DIGIT(buffer);
      return buffer + 1;
    }

    // Magnitude must be 2 or larger initially because a decimal
    // point cannot be placed /between/ a single digit, so we can
    // skip the initial check.
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

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [46] */, float value) {
    jkj::dragonbox::float_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<float>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          float, jkj::dragonbox::default_float_traits<float>
        >(significandBits, exponentBits);

        // An exponent of 0 means the decimal point is at the right end of the number,
        std::size_t digitCountMinusOne = (
          Nuclex::Support::BitTricks::GetLogBase10(result.significand)
        );
        int decimalPointPosition = result.exponent + digitCountMinusOne;

        return buffer;

        // The decimal point is *before* the entire number, i.e. 0.123 or 0.0123,
        // so we need to start with a '0.', possibly followed by more zeros
        if(digitCountMinusOne < (-result.exponent)) {

          // The exponent is negative, so this number is less than 1.0 and needs to begin with
          // a "0." and possibly additional zeros.
          *buffer++ = u8'0';
          *buffer++ = u8'.';
          while(result.exponent + digitCountMinusOne < -1) {
            *buffer++ = u8'0';
            ++result.exponent;
          }

          // The decimal point has already been placed, so the remaining digits can
          // be appended as-is.
          // PERF: We know the digit count already. Would a manual jump table be faster?
          char *shit = FormatInteger(buffer, result.significand);
          return formatIntegerSimple(buffer, result.significand, digitCountMinusOne);
          //return FormatInteger(buffer, result.significand);

        } else if(decimalPointPosition < digitCountMinusOne) {
        } else {

          //
          // TODO: Convert result into string
          //buffer = writeDigits(buffer, result.significand, )

        //} else { // exponent matches digitCount, so the decimal point is exactly at the end

          // The number is an integer. Per convention, we write it like 123.0
          buffer = FormatInteger(buffer, result.significand);;
          buffer[0] = u8'.';
          buffer[1] = u8'0';
          return buffer + 2;

        }

      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else {
      if(significandBits.has_all_zero_significand_bits()) {
        if(significandBits.is_negative()) {
          std::memcpy(buffer, "-Infinity", 9);
          return buffer + 9;
        } else {
          std::memcpy(buffer, "Infinity", 8);
          return buffer + 8;
        }
      } else {
        std::memcpy(buffer, "NaN", 3);
        return buffer + 3;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [325] */, double value) {
    jkj::dragonbox::float_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<double>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          double, jkj::dragonbox::default_float_traits<double>
        >(significandBits, exponentBits);

        // TODO: Convert result into string
        return FormatInteger(buffer, result.significand);

      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else {
      if(significandBits.has_all_zero_significand_bits()) {
        if(significandBits.is_negative()) {
          std::memcpy(buffer, "-Infinity", 9);
          return buffer + 9;
        } else {
          std::memcpy(buffer, "Infinity", 8);
          return buffer + 8;
        }
      } else {
        std::memcpy(buffer, "NaN", 3);
        return buffer + 3;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#undef WRITE_TWO_DIGITS
#undef WRITE_ONE_DIGIT
#undef READY_NEXT_TWO_DIGITS
#undef PREPARE_NUMBER_OF_MAGNITUDE
