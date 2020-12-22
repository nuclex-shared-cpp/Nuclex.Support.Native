#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Text/LexicalAppend.h"

#include "Dragon4/PrintFloat.h"
#include "Erthink/erthink_u2a.h"
#include "Ryu/ryu_parse.h"

#include <limits> // for std::numeric_limits

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

} // anonmymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::string &target, const bool &from) {
    static const std::string trueString(u8"true");
    static const std::string falseString(u8"false");

    if(from) {
      target.append(trueString);
    } else {
      target.append(falseString);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const bool &from
  ) {
    if(from) {
      if(availableBytes >= 4U) {
        *target++ = 't';
        *target++ = 'r';
        *target++ = 'u';
        *target = 'e';
      }
      return 4U;
    } else {
      if(availableBytes >= 5U) {
        *target++ = 'f';
        *target++ = 'a';
        *target++ = 'l';
        *target++ = 's';
        *target = 'e';
      }
      return 5U;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void lexical_append(std::string &target, const char *from) {
    static const std::string nullString(u8"<nullptr>");

    if(from == nullptr) {
      target.append(nullString);
    } else {
      target.append(from);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t lexical_append(
    char *target, std::size_t availableBytes, const char *from
  ) {

    // If we've gotten a null pointer, append a special string indicating so
    if(from == nullptr) {
      if(availableBytes >= 9U) {
        *target++ = '<';
        *target++ = 'n';
        *target++ = 'u';
        *target++ = 'l';
        *target++ = 'l';
        *target++ = 'p';
        *target++ = 't';
        *target++ = 'r';
        *target = '>';
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

  template<> void lexical_append<>(std::string &target, const std::string &from) {
    target.append(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::string &from
  ) {
    std::size_t fromLength = from.length();
    if(fromLength > availableBytes) {
      return fromLength;
    }

    const char *fromBytes = from.c_str();
    for(std::size_t index = 0; index < fromLength; ++index) {
      target[index] = fromBytes[index];
    }

    return fromLength;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::string &target, const std::uint8_t &from) {
    std::string::size_type length = target.length();
    target.resize(length + countDigits(from));
    erthink::u2a(static_cast<std::uint32_t>(from), target.data() + length);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::uint8_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      erthink::u2a(static_cast<std::uint32_t>(from), target);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void lexical_append<>(std::string &target, const std::int8_t &from) {
    std::string::size_type length = target.length();
    target.resize(length + countDigits(from));
    erthink::i2a(static_cast<std::int32_t>(from), target.data() + length);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::int8_t &from
  ) {
    std::size_t requiredBytes = countDigits(from);
    if(availableBytes >= requiredBytes) {
      erthink::i2a(static_cast<std::int32_t>(from), target);
    }

    return requiredBytes;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
