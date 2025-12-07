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

#include "Nuclex/Support/Text/LexicalAppend.h"
#include "Nuclex/Support/BitTricks.h"

#include "./NumberFormatter.h"

#include <limits> // for std::numeric_limits
#include <algorithm> // for std::copy_n()

// TODO lexical_append() with std::u8string could resize within NumberFormatter
//
// The NumberFormatter already figures out the number of digits that need to be appended
// ahead of time, so the call to BitTricks::GetLogBase10() is completely redundant.
//
// Unclear if it's worth the effort, as the call is just one machine code instruction
// followed by a multiply and shift.
//

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of digits in a value</summary>
  /// <param name="value">Value for which the printed digits will be counted</param>
  /// <returns>The number of digits the value has when printed</returns>
  std::size_t countDigits(std::uint8_t value) {
    if(value < 10U) {
      return 1U;
    } else if(value < 100U) {
      return 2U;
    } else {
      return 3U;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of digits in a value</summary>
  /// <param name="value">Value for which the printed digits will be counted</param>
  /// <returns>The number of digits the value has when printed</returns>
  std::size_t countDigits(std::int8_t value) {
    if(value < 0) {
      if(value > -10) {
        return 2U;
      } else if(value > -100) {
        return 3U;
      } else {
        return 4U;
      }
    } else {
      if(value < 10) {
        return 1U;
      } else if(value < 100) {
        return 2U;
      } else {
        return 3U;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of digits in a value</summary>
  /// <param name="value">Value for which the printed digits will be counted</param>
  /// <returns>The number of digits the value has when printed</returns>
  std::size_t countDigits(std::uint16_t value) {
    if(value < 10U) {
      return 1U;
    } else if(value < 100U) {
      return 2U;
    } else if(value < 1000U) {
      return 3U;
    } else if(value < 10000U) {
      return 4U;
    } else {
      return 5U;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of digits in a value</summary>
  /// <param name="value">Value for which the printed digits will be counted</param>
  /// <returns>The number of digits the value has when printed</returns>
  std::size_t countDigits(std::int16_t value) {
    if(value < 0) {
      if(value > -10) {
        return 2U;
      } else if(value > -100) {
        return 3U;
      } else if(value > -1000) {
        return 4U;
      } else if(value > -10000) {
        return 5U;
      } else {
        return 6U;
      }
    } else {
      if(value < 10) {
        return 1U;
      } else if(value < 100) {
        return 2U;
      } else if(value < 1000) {
        return 3U;
      } else if(value < 10000) {
        return 4U;
      } else {
        return 5U;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonmymous namespace

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const bool &from) {
    static const std::u8string trueString(u8"true");
    static const std::u8string falseString(u8"false");

    if(from) {
      target.append(trueString);
    } else {
      target.append(falseString);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const bool &from
  ) {
    if(from) {
      if(availableBytes >= 4U) {
        *target++ = u8't';
        *target++ = u8'r';
        *target++ = u8'u';
        *target = u8'e';
      }
      return 4U;
    } else {
      if(availableBytes >= 5U) {
        *target++ = u8'f';
        *target++ = u8'a';
        *target++ = u8'l';
        *target++ = u8's';
        *target = u8'e';
      }
      return 5U;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void lexical_append(std::u8string &target, const char8_t *from) {
    static const std::u8string nullString(u8"<nullptr>");

    if(from == nullptr) {
      target.append(nullString);
    } else {
      target.append(from);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t lexical_append(
    char8_t *target, std::size_t availableBytes, const char8_t *from
  ) {

    // If we've gotten a null pointer, append a special string indicating so
    if(from == nullptr) {
      if(availableBytes >= 9U) {
        *target++ = u8'<';
        *target++ = u8'n';
        *target++ = u8'u';
        *target++ = u8'l';
        *target++ = u8'l';
        *target++ = u8'p';
        *target++ = u8't';
        *target++ = u8'r';
        *target = u8'>';
      }

      return 9U;
    }

    std::size_t fromByteCount = 0;

    // Copy bytes one by one, scanning for the terminating zero byte
    while(fromByteCount < availableBytes) {
      char current = from[fromByteCount];
      if(current == '\0') {
        return fromByteCount;
      }

      target[fromByteCount] = current;
      ++fromByteCount;
    }

    // If this point is reached, there was not enough space available
    // and we have to complete scanning the source string so we can deliver
    // the required length to the caller
    while(from[fromByteCount] != '\0') {
      ++fromByteCount;
    }

    return fromByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::u8string &from) {
    target.append(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::u8string &from
  ) {
    std::size_t fromLength = from.length();
    if(fromLength > availableBytes) {
      return fromLength;
    }

    const char8_t *fromBytes = from.c_str();
    for(std::size_t index = 0; index < fromLength; ++index) {
      target[index] = fromBytes[index];
    }

    return fromLength;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::uint8_t &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + countDigits(from));
    FormatInteger(target.data() + length, from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::uint8_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::int8_t &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + countDigits(from));
    FormatInteger(target.data() + length, from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::int8_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::uint16_t &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + countDigits(from));
    FormatInteger(target.data() + length, from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::uint16_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::int16_t &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + countDigits(from));
    FormatInteger(target.data() + length, from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::int16_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::uint32_t &from) {
    std::u8string::size_type length = target.length();
    if(from >= 1) {
      target.resize(length + BitTricks::GetLogBase10(from) + 1);
      FormatInteger(target.data() + length, from);
    } else {
      target.push_back('0');
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::uint32_t &from
  ) {
    std::size_t requiredBytes = (from >= 1) ? (BitTricks::GetLogBase10(from) + 1) : 1;
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::int32_t &from) {
    std::u8string::size_type length = target.length();
    if(from >= 1) {
      target.resize(length + BitTricks::GetLogBase10(static_cast<std::uint32_t>(from)) + 1);
      FormatInteger(target.data() + length, static_cast<std::uint32_t>(from));
    } else if(from == 0) {
      target.push_back('0');
    } else {
      target.resize(length + BitTricks::GetLogBase10(static_cast<std::uint32_t>(-from)) + 2);
      FormatInteger(target.data() + length, from);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::int32_t &from
  ) {
    if(from >= 1) {
      std::size_t requiredBytes = BitTricks::GetLogBase10(static_cast<std::uint32_t>(from)) + 1;
      if(availableBytes >= requiredBytes) {
        FormatInteger(target, static_cast<std::uint32_t>(from));
      }
      return requiredBytes;
    } else if(from == 0) {
      if(availableBytes >= 1U) {
        target[0] = '0';
      }
      return 1U;
    } else {
      std::size_t requiredBytes = BitTricks::GetLogBase10(static_cast<std::uint32_t>(-from)) + 2;
      if(availableBytes >= requiredBytes) {
        FormatInteger(target, from);
      }
      return requiredBytes;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::uint64_t &from) {
    std::u8string::size_type length = target.length();
    if(from >= 1) {
      target.resize(length + BitTricks::GetLogBase10(from) + 1);
      FormatInteger(target.data() + length, from);
    } else {
      target.push_back('0');
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::uint64_t &from
  ) {
    std::size_t requiredBytes = (from >= 1) ? (BitTricks::GetLogBase10(from) + 1) : 1;
    if(availableBytes >= requiredBytes) {
      FormatInteger(target, from);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const std::int64_t &from) {
    std::u8string::size_type length = target.length();
    if(from >= 1) {
      target.resize(length + BitTricks::GetLogBase10(static_cast<std::uint64_t>(from)) + 1);
      FormatInteger(target.data() + length, static_cast<std::uint64_t>(from));
    } else if(from == 0) {
      target.push_back('0');
    } else {
      target.resize(length + BitTricks::GetLogBase10(static_cast<std::uint64_t>(-from)) + 2);
      FormatInteger(target.data() + length, from);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const std::int64_t &from
  ) {
    if(from >= 1) {
      std::size_t requiredBytes = BitTricks::GetLogBase10(static_cast<std::uint64_t>(from)) + 1;
      if(availableBytes >= requiredBytes) {
        FormatInteger(target, static_cast<std::uint64_t>(from));
      }
      return requiredBytes;
    } else if(from == 0) {
      if(availableBytes >= 1U) {
        target[0] = '0';
      }
      return 1U;
    } else {
      std::size_t requiredBytes = BitTricks::GetLogBase10(static_cast<std::uint64_t>(-from)) + 2;
      if(availableBytes >= requiredBytes) {
        FormatInteger(target, from);
      }
      return requiredBytes;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const float &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + 48U);

    char8_t *end = FormatFloat(target.data() + length, from);
    target.resize(end - target.data());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const float &from
  ) {
    if(availableBytes >= 48U) {
      char8_t *end = FormatFloat(target, from);
      return static_cast<std::size_t>(end - target);
    } else {
      char8_t characters[48];
      char8_t *end = FormatFloat(characters, from);

      std::size_t actualLength = static_cast<std::size_t>(end - characters);
      if(availableBytes >= actualLength) {
        std::copy_n(characters, actualLength, target);
      }

      return actualLength;
    }
  }


  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::u8string &target, const double &from) {
    std::u8string::size_type length = target.length();
    target.resize(length + 325U);

    char8_t *end = FormatFloat(target.data() + length, from);
    target.resize(end - target.data());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char8_t *target, std::size_t availableBytes, const double &from
  ) {
    if(availableBytes >= 325U) {
      char8_t *end = FormatFloat(target, from);
      return static_cast<std::size_t>(end - target);
    } else {
      char8_t characters[325];
      char8_t *end = FormatFloat(characters, from);

      std::size_t actualLength = static_cast<std::size_t>(end - characters);
      if(availableBytes >= actualLength) {
        std::copy_n(characters, actualLength, target);
      }

      return actualLength;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
