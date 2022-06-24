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

#include "./DragonBox-1.1.2/dragonbox.h" // for the float-to-decimal algorithm
#include "Nuclex/Support/BitTricks.h" // for the base-10 log function

#include <cstdlib>
#include <cstring> // for std::memcpy()

// https://quick-bench.com/q/8j_Lm35goVp7YjFtQ-BDpg6zFRg
//
// JEAIII optimized surprisingly well (no div/mul?!)
// https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGEgMykrgAyeAyYAHI%2BAEaYxCD%2BABykAA6oCoRODB7evgGp6ZkCoeFRLLHxSbaY9o4CQgRMxAQ5Pn5cgXaYDlkNTQQlkTFxCckKjc2teR22EwNhQ%2BUjSQCUtqhexMjsHOb%2BYcjeWADUJv5uyOPEYcBn2CYaAIJ7B0eYp%2BeXBPiCdw/PZn2DEOXhOZwuLCYBAQfyeL2Bbw%2BFzCBAIAE8UpgFLCAUCQWDPuN0LQ8NEcf9xlC8MhjmgGONMKoUsRaQgmsdiEx8KoAPpcDQaHmNaL0EwAVisYoAIh8ZSYAOxWJ7HFXHMAcDTq0hqjVanWa3g6rh69UG7Xqswm3WG9X%2BK1mnWSLX/VX6q1i%2B1WgBsnptHHlvvNHESgZ1AE5ncrVerjX6HTGrbGg0mdZa/SnbYmrU7eC7o9x3Vm/T701aA6W/SGK0GI7moyqLaHG9XU1a00H2zq7X7O%2Bqc6Q8w2OL2OB6e9622XJ5XpzXI49XZm41bu8mV7Ou%2Bu/avHfPFxwd%2Bqx0HDxwSyep9urVWL37awP67vl37%2B0bsxu%2B1ug6/P3WF/mf1Hd9i2AoNy2/a9QPDPd82PN0/TghMEI/ICr2Ql8YKHRDUKDbDzx1bDwIIyD0LnP993w00JxbdVKOHajbyDOj%2B0HHU6Lwhi2MvJiSJ4u9MJ1IiqL9ISCxElDRNPUSWMfdVRMIzi5O4wTeJU/jyPzG94KDLSkJ0lDdIPVTfwff8h0M7DDLowzRMMuyrXvVj1XvbToJojgXObMjGLciCNKHTycN8rj1LAhzjI88LdjheUpTOJVnieZA2WIAAqY5CFQJghQAd1QHl8GAQgFB5TFiAKvAADcICJEAQC8FF/DMIVjgYbVkqadLoi8KgqDiFZTkVPMOpZAhMBYFJxQsRJxTi/wEpVEb0pSAgWTOGUxom05LGODIAC9MFQKgIE2lIVnivNapAfBKpayqxC8d51sGixjg0bUGEGuaFuOHKEDod4IE%2Bv5/BlfkNAGhUfpVFaWQAWmeswLsfe7vEwAA6YhxtlY5Uce9GAEcvFQAhtrFY5weRszcYejGiZJnG8bp4nSYAegpgUqddK62BYZAUjRCBYe1TluT5AUhSYEUnp2iAmcx8aBvSsxtTMc75tY4HQZptHCZZrmvrzPAqGOIGPnubX%2BUhobHx58b%2BcF7reriEWuTwXlwcl6Xttez7ldV9XoeOJ2%2BrWyxEYNhU4qjGolBe1iQ7iKaNFmnGRogYSfdawOE560OfeerhI9ivMmC8IgOXGpgwhuHl6CMaEcdOrP9sO47ToGuHjlhg27b5gWIETshu9WkWq5row68YYBoRzqMsYITZPqHrOschCfgCnhuYQ1mLo8Sx4UWOdeGAga2g5G44xtUAgpq4JHpRxqGU5L3fqcy7KCDyiqioIEqyoqtVTkDB0Bn21NfAgc9nixQ4GsWgnAxS8D8BqXgqBOBuGsNYXaGwtgy38DwUgBBNCwLWAAaxAGKd68COCSCQcQ0gaCOC8AUCAd6RCUFrDgLAJAaAJoAzIBQCAvCUj8JAMAe%2BXA%2BB0DGsQFhg96HRDCE0NEnACG8LYIIAA8gwWgKiUGkCwJCIw4h9H4Cxj0SqWJ6GMm6OXHYBCUQ1HoSSaInJiBog8Fgehq08AsFUbAvgBhgAKAAGp4EwDlTRmJkEEP4IIEQYh2BSBkIIRQKh1D6N0JIgwRgUCYMsPoUkLDIBrFQCtLILCOBw00f4ZhNRuh1D8BAVwUx2hBBAYMMoFQ9BpAyI01pPTCiNM6cMeIkiug9HqHMAZ4z6mTIYH0ZoIylhjNmP0GZaylkLC6SMLgawFA4O2HoVamB7EBOoYg0gyCtAMM4KoRIXo4ZekkMcYAyAaT33RlwU2GDw4FOOLgTKYd8Hag8Hw%2BgwK9m8HYVoFYZCKFUM4LQq59DGHMNYYQ4hcL9CcDMHQ/RaLMUcLWJY2RWQQCSCAA%3D%3D

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
  #define PREPARE_NUMBER_OF_MAGNITUDE(bufferPointer, totalLength) \
    t = ( \
      (std::uint64_t(1) << (32 + totalLength / 5 * totalLength * 53 / 16)) / \
      std::uint32_t(1e##totalLength) + 1 + totalLength/6 - totalLength/8 \
    ), \
    t *= u, \
    t >>= totalLength / 5 * totalLength * 53 / 16, \
    t += totalLength / 6 * 4

  // Brings the next two digits of the prepeared number into the upper 32 bits
  // so they can be extracted by the WRITE_ONE_DIGIT and WRITE_TWO_DIGITS macros
  #define PREPARE_NEXT_TWO_DIGITS() \
    t = std::uint64_t(100) * static_cast<std::uint32_t>(t)

  // Appends the next two highest digits in the prepared number to the char buffer
  // Also adjusts the number such that the next two digits are ready for extraction.
  #define WRITE_TWO_DIGITS(bufferPointer) \
    *reinterpret_cast<TwoChars *>(bufferPointer) = ( \
      *reinterpret_cast<const TwoChars *>(&Radix100[(t >> 31) & 0xFE]) \
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
  inline void appendDigits32(char *&buffer, std::uint32_t u) {
    std::uint64_t t;

    // It appears that the branching tree in the jeaiii implementation beats this.
    if(u < 100) {
      if(u < 10) {
        *buffer++ = u8'0' + u;
      } else {
        *reinterpret_cast<TwoChars *>(buffer) = (
          *reinterpret_cast<const TwoChars *>(&Radix100[u * 2])
        );
        buffer += 2;
      }
    } else if(u < 1'000'000) {
      if(u < 10'000) {
        if(u < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          buffer += 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 2);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          buffer += 4;
        }
      } else {
        if(u < 100'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 3);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          WRITE_ONE_DIGIT(buffer + 4);
          buffer += 5;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 4);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          buffer += 6;
        }
      }
    } else {
      if(u < 100'000'000) {
        if(u < 10'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 5);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          WRITE_ONE_DIGIT(buffer + 6);
          buffer += 7;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 6);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          buffer += 8;
        }
      } else {
        if(u < 1'000'000'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 7);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          WRITE_ONE_DIGIT(buffer + 8);
          buffer += 9;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 8);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 4);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 6);
          PREPARE_NEXT_TWO_DIGITS();
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
  inline void appendDigits64(char *&buffer, std::uint64_t n) {

    // If this number fits into 32 bits, then don't bother with the extra processing
    std::uint32_t u = static_cast<std::uint32_t>(n);
    if(u == n) {
      appendDigits32(buffer, u);
      return;
    }

    // Temporary value, the integer to be converted will be placed in the upper end
    // of its lower 32 bits and then converted by shifting 2 characters apiece into
    // the upper 32 bits of this 64 bit integer.
    std::uint64_t t;

    std::uint64_t a = n / 100'000'000u;
    u = static_cast<std::uint32_t>(a);
    if(u == a) {
      appendDigits32(buffer, u);
    } else {
      u = static_cast<std::uint32_t>(a / 100'000'000u);

      if(u < 100) {
        if(u < 10) {
          *buffer++ = u8'0' + u;
        } else {
          *reinterpret_cast<TwoChars *>(buffer) = (
            *reinterpret_cast<const TwoChars *>(&Radix100[u * 2])
          );
          buffer += 2;
        }
      } else {
        if(u < 1'000) {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 1);
          WRITE_TWO_DIGITS(buffer);
          WRITE_ONE_DIGIT(buffer + 2);
          buffer += 3;
        } else {
          PREPARE_NUMBER_OF_MAGNITUDE(buffer, 2);
          WRITE_TWO_DIGITS(buffer);
          PREPARE_NEXT_TWO_DIGITS();
          WRITE_TWO_DIGITS(buffer + 2);
          buffer += 4;
        }
      }

      u = a % 100'000'000u;

      PREPARE_NUMBER_OF_MAGNITUDE(buffer, 6);
      WRITE_TWO_DIGITS(buffer);
      PREPARE_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 2);
      PREPARE_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 4);
      PREPARE_NEXT_TWO_DIGITS();
      WRITE_TWO_DIGITS(buffer + 6);
      buffer += 8;
    }

    u = n % 100'000'000u;

    PREPARE_NUMBER_OF_MAGNITUDE(buffer, 6);
    WRITE_TWO_DIGITS(buffer);
    PREPARE_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 2);
    PREPARE_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 4);
    PREPARE_NEXT_TWO_DIGITS();
    WRITE_TWO_DIGITS(buffer + 6);
    buffer += 8;
  }

  #undef WRITE_TWO_DIGITS
  #undef WRITE_ONE_DIGIT
  #undef PREPARE_NEXT_TWO_DIGITS
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

  char *FormatInteger(std::uint32_t value, char *buffer /* [10] */) {
    appendDigits32(buffer, value);

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(std::int32_t value, char *buffer /* [11] */) {
    if(value < 0) {
      *buffer++ = u8'-';
      appendDigits32(buffer, absToUnsigned(value));
    } else {
      appendDigits32(buffer, static_cast<std::uint32_t>(value));
    }

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(std::uint64_t value, char *buffer /* [20] */) {
    appendDigits64(buffer, value);

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatInteger(std::int64_t value, char *buffer /* [20] */) {
    if(value < 0) {
      *buffer++ = u8'-';
      appendDigits64(buffer, absToUnsigned(value));
    } else {
      appendDigits64(buffer, static_cast<std::uint64_t>(value));
    }

    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(float value, char *buffer /* [46] */) {
    (void)value;
    (void)buffer;
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(double value, char *buffer /* [325] */) {
    (void)value;
    (void)buffer;
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
