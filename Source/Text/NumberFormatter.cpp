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

  struct pair { char t, o; };

  static const pair s_pairs[] = {
    { u8'0', u8'0' }, { u8'0', u8'1' }, { u8'0', u8'2' }, { u8'0', u8'3' }, { u8'0', u8'4' },
    { u8'0', u8'5' }, { u8'0', u8'6' }, { u8'0', u8'7' }, { u8'0', u8'8' }, { u8'0', u8'9' },
    { u8'1', u8'0' }, { u8'1', u8'1' }, { u8'1', u8'2' }, { u8'1', u8'3' }, { u8'1', u8'4' },
    { u8'1', u8'5' }, { u8'1', u8'6' }, { u8'1', u8'7' }, { u8'1', u8'8' }, { u8'1', u8'9' },
    { u8'2', u8'0' }, { u8'2', u8'1' }, { u8'2', u8'2' }, { u8'2', u8'3' }, { u8'2', u8'4' },
    { u8'2', u8'5' }, { u8'2', u8'6' }, { u8'2', u8'7' }, { u8'2', u8'8' }, { u8'2', u8'9' },
    { u8'3', u8'0' }, { u8'3', u8'1' }, { u8'3', u8'2' }, { u8'3', u8'3' }, { u8'3', u8'4' },
    { u8'3', u8'5' }, { u8'3', u8'6' }, { u8'3', u8'7' }, { u8'3', u8'8' }, { u8'3', u8'9' },
    { u8'4', u8'0' }, { u8'4', u8'1' }, { u8'4', u8'2' }, { u8'4', u8'3' }, { u8'4', u8'4' },
    { u8'4', u8'5' }, { u8'4', u8'6' }, { u8'4', u8'7' }, { u8'4', u8'8' }, { u8'4', u8'9' },
    { u8'5', u8'0' }, { u8'5', u8'1' }, { u8'5', u8'2' }, { u8'5', u8'3' }, { u8'5', u8'4' },
    { u8'5', u8'5' }, { u8'5', u8'6' }, { u8'5', u8'7' }, { u8'5', u8'8' }, { u8'5', u8'9' },
    { u8'6', u8'0' }, { u8'6', u8'1' }, { u8'6', u8'2' }, { u8'6', u8'3' }, { u8'6', u8'4' },
    { u8'6', u8'5' }, { u8'6', u8'6' }, { u8'6', u8'7' }, { u8'6', u8'8' }, { u8'6', u8'9' },
    { u8'7', u8'0' }, { u8'7', u8'1' }, { u8'7', u8'2' }, { u8'7', u8'3' }, { u8'7', u8'4' },
    { u8'7', u8'5' }, { u8'7', u8'6' }, { u8'7', u8'7' }, { u8'7', u8'8' }, { u8'7', u8'9' },
    { u8'8', u8'0' }, { u8'8', u8'1' }, { u8'8', u8'2' }, { u8'8', u8'3' }, { u8'8', u8'4' },
    { u8'8', u8'5' }, { u8'8', u8'6' }, { u8'8', u8'7' }, { u8'8', u8'8' }, { u8'8', u8'9' },
    { u8'9', u8'0' }, { u8'9', u8'1' }, { u8'9', u8'2' }, { u8'9', u8'3' }, { u8'9', u8'4' },
    { u8'9', u8'5' }, { u8'9', u8'6' }, { u8'9', u8'7' }, { u8'9', u8'8' }, { u8'9', u8'9' },
  };

  #define WRITE_PAIR(bufferIndex, pairIndex) *(pair*)&buffer[bufferIndex] = s_pairs[pairIndex]

  #define A(N) t = \
    ( \
      (std::uint64_t(1) << (32 + N / 5 * N * 53 / 16)) / \
      std::uint32_t(1e##N) + 1 + N/6 - N/8 \
    ), \
    t *= u, \
    t >>= N / 5 * N * 53 / 16, \
    t += N / 6 * 4, \
    WRITE_PAIR(0, t >> 32)

  #define S(bufferIndex) \
    buffer[bufferIndex] = char(std::uint64_t(10) * std::uint32_t(t) >> 32) + '0'

  #define D(bufferIndex) t = \
    std::uint64_t(100) * std::uint32_t(t), \
    WRITE_PAIR(bufferIndex, t >> 32)

  #define C0 buffer[0] = char(u) + '0'
  #define C1 WRITE_PAIR(0, u)
  #define C2 A(1), S(2)
  #define C3 A(2), D(2)
  #define C4 A(3), D(2), S(4)
  #define C5 A(4), D(2), D(4)
  #define C6 A(5), D(2), D(4), S(6)
  #define C7 A(6), D(2), D(4), D(6)
  #define C8 A(7), D(2), D(4), D(6), S(8)
  #define C9 A(8), D(2), D(4), D(6), D(8)

  #define LENGTH_0_TO_9(F) u <         100 ? LENGTH_0_OR_1(F) : LENGTH_2_TO_9(F)
  #define LENGTH_2_TO_9(F) u <   1'000'000 ? LENGTH_2_TO_5(F) : LENGTH_6_TO_9(F)
  #define LENGTH_2_TO_5(F) u <      10'000 ? LENGTH_2_OR_3(F) : LENGTH_4_OR_5(F)
  #define LENGTH_6_TO_9(F) u < 100'000'000 ? LENGTH_6_OR_7(F) : LENGTH_8_OR_9(F)
  #define LENGTH_0_TO_3(F) u <         100 ? LENGTH_0_OR_1(F) : LENGTH_2_OR_3(F)

  #define LENGTH_0_OR_1(F) u <            10 ? F(0) : F(1)
  #define LENGTH_2_OR_3(F) u <         1'000 ? F(2) : F(3)
  #define LENGTH_4_OR_5(F) u <       100'000 ? F(4) : F(5)
  #define LENGTH_6_OR_7(F) u <    10'000'000 ? F(6) : F(7)
  #define LENGTH_8_OR_9(F) u < 1'000'000'000 ? F(8) : F(9)

  #define PART(N) (C##N, buffer += N + 1)

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends the decimal digits for a written-out integer to a buffer</summary>
  /// <param name="buffer">
  ///   Buffer to which the decimal digits will be appended. Will be advanced to
  ///   a position one past that last character written.
  /// </param>
  /// <param name="n">Integer whose decimal digits will be appended</param>
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
  void appendDigits32(char *&buffer, std::uint32_t u) {
    std::uint64_t t;

    if(u < 100) {
      if(u < 10) {
        *buffer++ = u8'0' + u;
      } else {
        *buffer++ = Radix100[u * 2];
        *buffer++ = Radix100[u * 2] + 1;
      }
    } else if(u < 1'000'000) {
      if(u < 10'000) {
        if(u < 1'000) {
          C2;
          buffer += 3;
        } else {
          C3;
          buffer += 4;
        }
      } else {
        if(u < 100'000) {
          C4;
          buffer += 5;
        } else {
          C5;
          buffer += 6;
        }
      }
    } else {
      if(u < 100'000'000) {
        if(u < 10'000'000) {
          C6;
          buffer += 7;
        } else {
          C7;
          buffer += 8;
        }
      } else {
        if(u < 1'000'000'000) {
          C8;
          buffer += 9;
        } else {
          C9;
          buffer += 10;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends the decimal digits for a written-out integer to a buffer</summary>
  /// <param name="buffer">
  ///   Buffer to which the decimal digits will be appended. Will be advanced to
  ///   a position one past that last character written.
  /// </param>
  /// <param name="n">Integer whose decimal digits will be appended</param>
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
  void appendDigits64(char *&buffer, std::uint64_t n) {
    std::uint64_t t;

    std::uint32_t u = static_cast<std::uint32_t>(n);
    if(u == n) {
      appendDigits32(buffer, u);
      return;
    }

    std::uint64_t a = n / 100'000'000u;
    u = static_cast<std::uint32_t>(a);

    if(u == a) {
      appendDigits32(buffer, u);
    } else {
      u = static_cast<std::uint32_t>(a / 100'000'000u);
      LENGTH_0_TO_3(PART);
      u = a % 100'000'000u;
      C7;
      buffer += 8;
    }

    u = n % 100'000'000u;

    C7;
    buffer += 8;
  }

  #undef LAST
  #undef PART

  #undef LENGTH_8_OR_9
  #undef LENGTH_6_OR_7
  #undef LENGTH_4_OR_5
  #undef LENGTH_2_OR_3
  #undef LENGTH_0_OR_1

  #undef LENGTH_0_TO_3
  #undef LENGTH_6_TO_9
  #undef LENGTH_2_TO_5
  #undef LENGTH_2_TO_9
  #undef LENGTH_0_TO_9

  #undef C9
  #undef C8
  #undef C7
  #undef C6
  #undef C5
  #undef C4
  #undef C3
  #undef C2
  #undef C1
  #undef C0

  #undef D
  #undef S
  #undef A
  #undef WRITE_PAIR
  #undef P

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

  char *itoa_better_y(std::uint32_t n, char *buffer) {
    std::uint64_t prod;

    auto get_next_two_digits = [&]() {
      prod = std::uint32_t(prod) * std::uint64_t(100);
      return int(prod >> 32);
    };
    auto print_1 = [&](int digit) {
      buffer[0] = char(digit + '0');
      buffer += 1;
    };
    auto print_2 = [&] (int two_digits) {
      std::memcpy(buffer, Radix100 + two_digits * 2, 2);
      buffer += 2;
    };
    auto print = [&](std::uint64_t magic_number, int extra_shift, auto remaining_count) {
      prod = n * magic_number;
      prod >>= extra_shift;
      auto two_digits = int(prod >> 32);

      if (two_digits < 10) {
        print_1(two_digits);
        for (int i = 0; i < remaining_count; ++i) {
          print_2(get_next_two_digits());
        }
      }
      else {
        print_2(two_digits);
        for (int i = 0; i < remaining_count; ++i) {
          print_2(get_next_two_digits());
        }
      }
    };

    if (n < 100) {
      if (n < 10) {
        // 1 digit.
        print_1(n);
      }
      else {
        // 2 digit.
        print_2(n);
      }
    }
    else {
      if (n < 100'0000) {
        if (n < 1'0000) {
          // 3 or 4 digits.
          // 42949673 = ceil(2^32 / 10^2)
          print(42949673, 0, std::integral_constant<int, 1>{});
        }
        else {
          // 5 or 6 digits.
          // 429497 = ceil(2^32 / 10^4)
          print(429497, 0, std::integral_constant<int, 2>{});
        }
      }
      else {
        if (n < 1'0000'0000) {
          // 7 or 8 digits.
          // 281474978 = ceil(2^48 / 10^6) + 1
          print(281474978, 16, std::integral_constant<int, 3>{});
        }
        else {
          if (n < 10'0000'0000) {
            // 9 digits.
            // 1441151882 = ceil(2^57 / 10^8) + 1
            prod = n * std::uint64_t(1441151882);
            prod >>= 25;
            print_1(int(prod >> 32));
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
          }
          else {
            // 10 digits.
            // 1441151881 = ceil(2^57 / 10^8)
            prod = n * std::uint64_t(1441151881);
            prod >>= 25;
            print_2(int(prod >> 32));
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
            print_2(get_next_two_digits());
          }
        }
      }
    }
    return buffer;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
