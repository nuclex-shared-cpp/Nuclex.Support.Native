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
//#include "Nuclex/Support/BitTricks.h" // for the base-10 log function

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
    t = ( \
      (std::uint64_t(1) << (32 + magnitude / 5 * magnitude * 53 / 16)) / \
      std::uint32_t(1e##magnitude) + 1 + magnitude/6 - magnitude/8 \
    ), \
    t *= number, \
    t >>= magnitude / 5 * magnitude * 53 / 16, \
    t += magnitude / 6 * 4

  // Brings the next two digits of the prepeared number into the upper 32 bits
  // so they can be extracted by the WRITE_ONE_DIGIT and WRITE_TWO_DIGITS macros
  #define READY_NEXT_TWO_DIGITS() \
    t = std::uint64_t(100) * static_cast<std::uint32_t>(t)

  // Appends the next two highest digits in the prepared number to the char buffer
  // Also adjusts the number such that the next two digits are ready for extraction.
  #define WRITE_TWO_DIGITS(bufferPointer) \
    *reinterpret_cast<TwoChars *>(bufferPointer) = ( \
      *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[(t >> 31) & 0xFE]) \
    )

  // Appends the next highest digit in the prepared number to the char buffer
  // Thus doesn't adjust the number because it is always used on the very last digit.
  #define WRITE_ONE_DIGIT(bufferPointer) \
    *reinterpret_cast<char *>(bufferPointer) = ( \
      u8'0' + static_cast<char>(std::uint64_t(10) * std::uint32_t(t) >> 32) \
    )

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends the character for an integral number in text form to a buffer</summary>
  /// <param name="buffer">
  ///   Buffer to which the character will be appended. Will be advanced to a position one past
  ///   the last character written.
  /// </param>
  /// <param name="u">Integer that will be appended to a buffer in textual form</param>
  /// <remarks>
  ///   <para>
  ///     This method does *NOT* write a closing zero byte as would be customary with C strings.
  ///   </para>
  ///   <para>
  ///     This is a direct clone of James Edward Anhalt III.'s itoa() implementation,
  ///     modified a little bit to only check for negative numbers if the input is unsigned,
  ///     also avoiding macros (if I succeed, note to self, remove this note)
  ///   </para>
  /// </remarks>
  inline void writeDigits32(char *&buffer, std::uint32_t u) {
    std::uint64_t t;

    // I have a nice Nuclex::Support::BitTricks::GetLogBase10() method which uses
    // no branching, just the CLZ (count leading zeros) CPU instruction, but feeding
    // this into a switch statement turns out to be slower than the branching tree.
    //
    // I also tested building a manual jump table with functions for each digit count
    // that is indexed by GetLogBase10() and called - so just one indirection in place
    // of several branching instructions, but it was slower, too. Not predictable enough
    // for the CPU?
    //
    // So this bunch of branches is outperforming every trick I have...
    //
    if(u < 100) {
      if(u < 10) {
        *buffer++ = u8'0' + u;
      } else {
        *reinterpret_cast<TwoChars *>(buffer) = (
          *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[u * 2])
        );
        buffer += 2;
      }
    } else if(u < 1'000'000) {
      if(u < 10'000) {
        if(u < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          buffer += 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 2);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          buffer += 4;
        }
      } else {
        if(u < 100'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 3);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          WRITE_ONE_DIGIT(buffer + 4);
          buffer += 5;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 4);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          buffer += 6;
        }
      }
    } else {
      if(u < 100'000'000) {
        if(u < 10'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 5);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          WRITE_ONE_DIGIT(buffer + 6);
          buffer += 7;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 6);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          buffer += 8;
        }
      } else {
        if(u < 1'000'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 7);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          WRITE_ONE_DIGIT(buffer + 8);
          buffer += 9;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 8);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 8);
          buffer += 10;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends the character for an integral number in text form to a buffer</summary>
  /// <param name="buffer">
  ///   Buffer to which the character will be appended. Will be advanced to a position one past
  ///   the last character written.
  /// </param>
  /// <param name="n">Integer that will be appended to a buffer in textual form</param>
  /// <remarks>
  ///   <para>
  ///     This method does *NOT* write a closing zero byte as would be customary with C strings.
  ///   </para>
  ///   <para>
  ///     This is a direct clone of James Edward Anhalt III.'s itoa() implementation,
  ///     modified a little bit to only check for negative numbers if the input is unsigned,
  ///     also avoiding macros (if I succeed, note to self, remove this note)
  ///   </para>
  /// </remarks>
  inline void writeDigits64(char *&buffer, std::uint64_t n) {

    // If this number fits into 32 bits, then don't bother with the extra processing
    std::uint32_t u = static_cast<std::uint32_t>(n);
    if(u == n) {
      writeDigits32(buffer, u);
      return;
    }

    // Temporary value, the integer to be converted will be placed in the upper end
    // of its lower 32 bits and then converted by shifting 2 characters apiece into
    // the upper 32 bits of this 64 bit integer.
    std::uint64_t t;

    std::uint64_t a = n / 100'000'000u;
    u = static_cast<std::uint32_t>(a);
    if(u == a) {
      writeDigits32(buffer, u);
    } else {
      u = static_cast<std::uint32_t>(a / 100'000'000u);

      if(u < 100) {
        if(u < 10) {
          *buffer++ = u8'0' + u;
        } else {
          *reinterpret_cast<TwoChars *>(buffer) = (
            *reinterpret_cast<const TwoChars *>(&Nuclex::Support::Text::Radix100[u * 2])
          );
          buffer += 2;
        }
      } else {
        if(u < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          buffer += 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(u, 2);
          WRITE_TWO_DIGITS(buffer);
          READY_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          buffer += 4;
        }
      }

      u = a % 100'000'000u;

      PREPARE_NUMBER_OF_MAGNITUDE(u, 6);
      WRITE_TWO_DIGITS(buffer);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 2);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 4);
      READY_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 6);
      buffer += 8;
    }

    u = n % 100'000'000u;

    PREPARE_NUMBER_OF_MAGNITUDE(u, 6);
    WRITE_TWO_DIGITS(buffer);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 2);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 4);
    READY_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 6);
    buffer += 8;
  }

  #undef WRITE_TWO_DIGITS
  #undef WRITE_ONE_DIGIT
  #undef READY_NEXT_TWO_DIGITS
  #undef PREPARE_NUMBER_OF_MAGNITUDE

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Takes the absolute value of a signed 32 bit integer and returns it as unsigned
  /// </summary>
  /// <param name="value">Value whose absolute value will be returned as an unsigned type</param>
  /// <returns>The absolute value if the input integer as an unsigned integer</returns>
  /// <remarks>
  ///   This avoids the undefined result of std::abs() applied to the lowest possible integer.
  /// </remarks>
  inline constexpr std::uint32_t absToUnsigned(std::int32_t value) noexcept {
    return 0u - static_cast<std::uint32_t>(value);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Takes the absolute value of a signed 64 bit integer and returns it as unsigned
  /// </summary>
  /// <param name="value">Value whose absolute value will be returned as an unsigned type</param>
  /// <returns>The absolute value if the input integer as an unsigned integer</returns>
  /// <remarks>
  ///   This avoids the undefined result of std::abs() applied to the lowest possible integer.
  /// </remarks>
  inline constexpr std::uint64_t absToUnsigned(std::int64_t value) noexcept {
    return 0u - static_cast<std::uint64_t>(value);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [10] */, std::uint32_t value) {
    writeDigits32(buffer, value);
    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [11] */, std::int32_t value) {
    if(value < 0) {
      *buffer++ = u8'-';
      writeDigits32(buffer, absToUnsigned(value));
    } else {
      writeDigits32(buffer, static_cast<std::uint32_t>(value));
    }

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [20] */, std::uint64_t value) {
    writeDigits64(buffer, value);

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(char *buffer /* [20] */, std::int64_t value) {
    if(value < 0) {
      *buffer++ = u8'-';
      writeDigits64(buffer, absToUnsigned(value));
    } else {
      writeDigits64(buffer, static_cast<std::uint64_t>(value));
    }

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
