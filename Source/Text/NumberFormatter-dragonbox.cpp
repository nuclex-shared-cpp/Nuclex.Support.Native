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

// Gets the address of the
#define GET_TWO_DIGITS_ADDRESS() \
  *reinterpret_cast<const char *>(&Nuclex::Support::Text::Radix100[(temp >> 31) & 0xFE])

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

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Factors the jeaiii algorithm uses to prepare a number for printing</summary>
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

  /// <summary>Bit shifts the jeaiii algorithm uses to prepare a number for printing</summary>
  const int shift[] = {
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

  /// <summary>Bias added to numbers by jeaiii algorithm</summary>
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

  char *formatIntegerWithDecimalPoint(
    char *buffer /* [10] */, std::uint32_t number, int magnitude, int decimalPointPosition
  ) {
    std::uint64_t temp = number;
    temp *= factors[magnitude];
    temp >>= shift[magnitude];
    temp += bias[magnitude];

    // The inputs are offset by 1 and incrementing them would just cost CPU cycles.
    //
    // 123.456    <-- magnitude = 5
    //    ^-- decimalPointPosition = 2
    //
    // So be aware!
    //

    // REMOVE - for testing
    char *originalBuffer = buffer;
    //if(decimalPointPosition > 0) {
    //  --decimalPointPosition;
    //}

    // TODO: We should be able to eliminate this case
    //       If this method is called, the decimal point is between two digits,
    //       thus the number must have magnitude 1 at least.
    if(magnitude == 0) {
      assert(false);
      WRITE_ONE_DIGIT(buffer);
      return buffer + 1;
    }

    // Calculate the remaining digits behind the decimal point
    magnitude -= decimalPointPosition;

    // Decimal point position indices *after* which digit the decimal point is to be placed,
    // so if it is zero we've got an odd number of digits before, otherwise an even number.
    if((decimalPointPosition & 1) == 0) {

      // Assumption: decimal point can not be at 0 because then it wouldn't be between
      // any digits and this method would not be called
      //assert((decimalPointPosition >= 1) && u8"Decimal point is inbetween other digits");

      // TODO
      return buffer;

    } else { // Number of digits before decimal point is even

      // Append the digits before the decimal point. We know it's an even number,
      // so we can skip the single digit check and don't need to store a half.
      for(;;) {
        WRITE_TWO_DIGITS(buffer);
        if(decimalPointPosition < 2) { // Are less than 2 remaining?
          buffer += 2;
          break;
        }
        READY_NEXT_TWO_DIGITS();
        decimalPointPosition -= 2;
        buffer += 2;
      }

      // Here comes the decimal point now
      *buffer++ = u8'.';

      // The digits behind the decimal point are at least 1 (otherwise this method
      // would not be called), but they may also be exactly 1, so deal with this here.
      if(magnitude == 1) {
        WRITE_ONE_DIGIT(buffer);
        return buffer + 1;
      }

      // Append the digits after the decimal point. This time we can use the ordinary
      // mixed double/single loop because we don't have to interrupt work in the middle.
      for(;;) {
        READY_NEXT_TWO_DIGITS();
        WRITE_TWO_DIGITS(buffer);
        if(magnitude < 4) { // Are less than 2 remaining? (4 because we didn't decrement yet)
          if(magnitude >= 3) { // is even 1 remaining? (3 because we didn't decrement yet)
            WRITE_ONE_DIGIT(buffer + 1);
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

        // If the exponent is negative, the decimal point lies within or before the number
        if(result.exponent < 0) {
          std::size_t digitCountMinusOne = (
            Nuclex::Support::BitTricks::GetLogBase10(result.significand)
          );
          int decimalPointPosition = result.exponent + digitCountMinusOne;

          // Does the decimal point lie before all the significand's digits?
          if(decimalPointPosition < 0) {
            buffer[0] = u8'0';
            buffer[1] = u8'.';
            buffer += 2;
            while(decimalPointPosition < -1) {
              *buffer++ = u8'0';
              ++decimalPointPosition;
            }
            return FormatInteger(buffer, result.significand);
          } else { // Nope, the decimal point is within the significand's digits!
            return formatIntegerWithDecimalPoint(
              buffer, result.significand, digitCountMinusOne, decimalPointPosition
            );
          }
        } else { // Exponent is zero or positive, number has no decimal places
          buffer = FormatInteger(buffer, result.significand);
          while(result.exponent > 0) {
            *buffer++ = u8'0';
            --result.exponent;
          }

          // Append a ".0" to indicate that this is a floating point number
          buffer[0] = u8'.';
          buffer[1] = u8'0';
          return buffer + 2;
        }
      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else if(significandBits.has_all_zero_significand_bits()) { // indicates infinity
      if(significandBits.is_negative()) {
        std::memcpy(buffer, "-Infinity", 9);
        return buffer + 9;
      } else {
        std::memcpy(buffer, "Infinity", 8);
        return buffer + 8;
      }
    } else { // infinite and non-empty signifiand -> not a number
      std::memcpy(buffer, "NaN", 3);
      return buffer + 3;
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

        // If the exponent is negative, the decimal point lies within or before the number
        if(result.exponent < 0) {
          std::size_t digitCountMinusOne = (
            Nuclex::Support::BitTricks::GetLogBase10(result.significand)
          );
          int decimalPointPosition = result.exponent + digitCountMinusOne;

          // Does the decimal point lie before all the significand's digits?
          if(decimalPointPosition < 0) {
            buffer[0] = u8'0';
            buffer[1] = u8'.';
            buffer += 2;
            while(decimalPointPosition < -1) {
              *buffer++ = u8'0';
              ++decimalPointPosition;
            }
            return FormatInteger(buffer, result.significand);
          } else { // Nope, the decimal point is within the significand's digits!
            //formatIntegerSimple(buffer, result.significand, digitCountMinusOne, decimalPointPosition);
            std::memcpy(buffer, "oh shit", 7);
            return buffer + 7;
          }

          // remove, for testing

        } else { // Exponent is zero or positive, number has no decimal places
          buffer = FormatInteger(buffer, result.significand);
          while(result.exponent > 0) {
            *buffer++ = u8'0';
            --result.exponent;
          }

          // Append a ".0" to indicate that this is a floating point number
          buffer[0] = u8'.';
          buffer[1] = u8'0';
          return buffer + 2;
        }
      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else if(significandBits.has_all_zero_significand_bits()) { // indicates infinity
      if(significandBits.is_negative()) {
        std::memcpy(buffer, "-Infinity", 9);
        return buffer + 9;
      } else {
        std::memcpy(buffer, "Infinity", 8);
        return buffer + 8;
      }
    } else { // infinite and non-empty signifiand -> not a number
      std::memcpy(buffer, "NaN", 3);
      return buffer + 3;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#undef WRITE_TWO_DIGITS
#undef WRITE_ONE_DIGIT
#undef READY_NEXT_TWO_DIGITS
#undef PREPARE_NUMBER_OF_MAGNITUDE
