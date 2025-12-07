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

#include "Nuclex/Support/Text/LexicalCast.h"
#include "./NumberFormatter.h"
#include "fast_float/fast_float.h"

#include <limits> // for std::numeric_limits
//#include <charconv> // for std::from_chars()

// Goal: print floating-point values accurately, locale-independent and without exponent
//
// std::to_string()
//   o No control over accuracy
//   o Locale-dependent (except Windows, std::setlocale(LC_NUMERIC, "de_DE.UTF-8"))
//   o Uses slow sprintf() internally
//
// Ryu https://github.com/ulfjack/ryu
//   o Always outputs exact number
//   o No control over exponential notation (and results such as "1E0" common)
//   o One of the fastest formatters at this time
//
// Dragon4 http://www.ryanjuckett.com
//   o Always outputs exact number
//   o Can force non-exponential notation
//   o Slower than typical libc implementations
//
// Grisu3 https://github.com/google/double-conversion
//   o Is not always exact
//
// Errol https://github.com/marcandrysco/Errol
//   o Alway outputs exact number
//   o No control over exponential notation
//
// DragonBox https://github.com/jk-jeon/dragonbox
//   o Always outputs exact number
//   o Fastest formatter around as of 2022
//   o Outputs two integers (value and exponent)
//

// Goal: print integral values accurately and fast
//   https://stackoverflow.com/questions/7890194/
//
// Terje http://computer-programming-forum.com/46-asm/7aa4b50bce8dd985.htm
//   o Fast
//
// Inge https://stackoverflow.com/questions/7890194/
//   o Faster
//
// Vitaut https://github.com/fmtlib/fmt
//   o Fastest
//
// Erthink https://github.com/leo-yuriev/erthink (defunct)
//   o Faster
//   o Handles signed and 64 bit integers
//   x Author has become a Putin/Dugin fascist
//
// Jeaiii https://github.com/jeaiii/itoa
//   o Fastest...est (better than Vitaut even)
//   o Handles signed and 64 bit integers
//
// Amartin https://github.com/amdn/itoa
//   o Fastest ever, apparently (benchmark says no :o)
//   o Handles signed and 64 bit integers
//   x Doesn't compile w/MSVC missing uint128_t
//

// Goal: convert ascii strings to floating point values
//   https://stackoverflow.com/questions/36018074
//
// Python https://github.com/enthought/Python-2.7.3/blob/master/Python/atof.c
//   o Unusable due to broken multiply/divide-by-10 technique
//
// Google Double-Conversion https://github.com/google/double-conversion
//   o Looks okay
//   o May rely on std::locale stuff
//
// Ryu https://github.com/ulfjack/ryu/blob/master/ryu/s2d.c
//   o Looks okay
//   o No locale stuff
//   o Used in unit test for full round-trip of all possible 32 bit floats
//
// Not checked:
//   o Ruby built-in - naive and broken multiply/divide-by-10 technique
//   o Sun - just as bad
//   o Lucent - may be okay, but sketchy implementation, doesn't build on GNU
//   o glibc - decent, but GPL
//

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Parses an integer from a span of UTF-8 characters</summary>>
  /// <typename name="TInteger">Integer type that will be parsed</typename>
  /// <param name="start">Pointer to the first number in the UTF-8 string</param>
  /// <param name="end">Pointer one past the last character to parse</param>
  template<typename TInteger>
  NUCLEX_SUPPORT_ALWAYS_INLINE TInteger integerFromUtf8(
    const char8_t *start, const char8_t *end
  ) {
    TInteger result = 0;
    fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(start, end, result);
    if(static_cast<bool>(outcome)) {
      return result;
    }

    // We intentionally discard the std::from_chars_result.
    // In case of an invalid input string, we silently fail and return 0.
    return TInteger(0);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Parses a floating point value from a span of UTF-8 characters</summary>>
  /// <typename name="TFloat">Floating point type that will be parsed</typename>
  /// <param name="start">Pointer to the first number in the UTF-8 string</param>
  /// <param name="end">Pointer one past the last character to parse</param>
  template<typename TFloat>
  NUCLEX_SUPPORT_ALWAYS_INLINE TFloat floatFromUtf8(const char8_t *start, const char8_t *end) {
    TFloat result = 0;
    fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(start, end, result);
    if(static_cast<bool>(outcome)) {
      return result;
    }

    // We intentionally discard the std::from_chars_result.
    // In case of an invalid input string, we silently fail and return 0.
    return TFloat(0);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const bool &from) {
    static const std::u8string trueString(u8"true");
    static const std::u8string falseString(u8"false");

    if(from) {
      return trueString;
    } else {
      return falseString;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return false;
    } else {
      return lexical_cast<bool>(std::u8string_view(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const std::u8string &from) {
    if(from.length() >= 4) {
      return (
        ((from[0] == u8't') || (from[0] == u8'T')) &&
        ((from[1] == u8'r') || (from[1] == u8'R')) &&
        ((from[2] == u8'u') || (from[2] == u8'U')) &&
        ((from[3] == u8'e') || (from[3] == u8'E'))
      );
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const std::u8string_view &from) {
    if(from.length() >= 4) {
      return (
        ((from[0] == u8't') || (from[0] == u8'T')) &&
        ((from[1] == u8'r') || (from[1] == u8'R')) &&
        ((from[2] == u8'u') || (from[2] == u8'U')) &&
        ((from[3] == u8'e') || (from[3] == u8'E'))
      );
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint8_t &from) {
    char8_t characters[4];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::uint8_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::uint8_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::uint8_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int8_t &from) {
    char8_t characters[5];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::int8_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::int8_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::int8_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint16_t &from) {
    char8_t characters[6];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::uint16_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::uint16_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::uint16_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int16_t &from) {
    char8_t characters[7];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::int16_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::int16_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::int16_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint32_t &from) {
    char8_t characters[11];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::uint32_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::uint32_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::uint32_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int32_t &from) {
    char8_t characters[12];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::int32_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::int32_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::int32_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint64_t &from) {
    char8_t characters[21];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::uint64_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::uint64_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::uint64_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int64_t &from) {
    char8_t characters[21];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      std::int64_t result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const std::u8string &from) {
    return integerFromUtf8<std::int64_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const std::u8string_view &from) {
    return integerFromUtf8<std::int64_t>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const float &from) {
    char8_t characters[48];
    char8_t *end = FormatFloat(characters, from);
    return std::u8string(characters, end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      float result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0.0f;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::u8string &from) {
    return floatFromUtf8<float>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::u8string_view &from) {
    return floatFromUtf8<float>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const double &from) {
    char8_t characters[325];
    char8_t *end = FormatFloat(characters, from);
    return std::u8string(characters, end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const char8_t *from) {
    if(from != nullptr) [[likely]] {
      double result;
      fast_float::from_chars_result_t<char8_t> outcome = fast_float::from_chars(
        from, (from + std::char_traits<char8_t>::length(from)), result
      );
      if(static_cast<bool>(outcome)) {
        return result;
      }
    }

    return 0.0;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::u8string &from) {
    return floatFromUtf8<double>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::u8string_view &from) {
    return floatFromUtf8<double>(from.data(), from.data() + from.length());
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
