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
#include "./Ryu/ryu_parse.h"

#include <limits> // for std::numeric_limits

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

  // TODO: This exists for performance reasons, remover and write custom integer parser

  /// <summary>Causes undefined behavior or converts a string to a long</summary>
  /// <param name="start">
  ///   Address of the first character of the null-terminated string that will be parsed
  /// </param>
  /// <returns>The unsigned long integer value parsed out of the string</returns>
  inline unsigned long u8strtoul(const char8_t *start) {
    return std::strtoul(
      reinterpret_cast<const char *>(start),
      nullptr, // end pointer
      10 // numeric base
    );
  }

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace Support { namespace Text {

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

  template<> bool lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return false;
    } else {
      return lexical_cast<bool>(std::u8string(from));
    }
  }

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

  template<> std::u8string lexical_cast<>(const std::uint8_t &from) {
    char8_t characters[4];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::uint8_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::uint8_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int8_t &from) {
    char8_t characters[5];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int8_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::int8_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint16_t &from) {
    char8_t characters[6];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::uint16_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::uint16_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int16_t &from) {
    char8_t characters[7];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int16_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::int16_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint32_t &from) {
    char8_t characters[11];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0U;
    } else {
      return static_cast<std::uint32_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::uint32_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int32_t &from) {
    char8_t characters[12];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int32_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::int32_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::uint64_t &from) {
    char8_t characters[21];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0ULL;
    } else {
      return static_cast<std::uint64_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::uint64_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const std::int64_t &from) {
    char8_t characters[21];
    const char8_t *end = FormatInteger(characters, from);
    return std::u8string(static_cast<const char8_t *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0LL;
    } else {
      return static_cast<std::int64_t>(u8strtoul(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const std::u8string &from) {
    return static_cast<std::int64_t>(u8strtoul(from.c_str()));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const float &from) {
    char8_t characters[48];
    char8_t *end = FormatFloat(characters, from);
    return std::u8string(characters, end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0.0f;
    } else {
      double result;
      enum ::Status status = ::s2d(from, &result);
      if(status == SUCCESS) {
        return static_cast<float>(result);
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::u8string &from) {
    double result;
    enum ::Status status = ::s2d_n(from.c_str(), static_cast<int>(from.length()), &result);
    if(status == SUCCESS) {
      return static_cast<float>(result);
    } else {
      return std::numeric_limits<float>::quiet_NaN();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string lexical_cast<>(const double &from) {
    char8_t characters[325];
    char8_t *end = FormatFloat(characters, from);
    return std::u8string(characters, end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const char8_t *from) {
    if(from == nullptr) {
      return 0.0;
    } else {
      double result;
      enum ::Status status = ::s2d(from, &result);
      if(status == SUCCESS) {
        return result;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::u8string &from) {
    double result;
    enum ::Status status = ::s2d_n(from.c_str(), static_cast<int>(from.length()), &result);
    if(status == SUCCESS) {
      return result;
    } else {
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
