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

#include "Nuclex/Support/Text/StringMatcher.h"
#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/Text/UnicodeHelper.h" // UTF encoding and decoding

#include <vector> // for std::vector
#include <stdexcept> // for std::invalid_argument
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw std::invalid_argument(
        reinterpret_cast<const char *>(
          u8"Illegal UTF-8 character(s) encountered"
        )
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the 32 bit murmur hash of a byte sequence</summary>
  /// <param name="key">Data for which the murmur hash will be calculated</param>
  /// <param name="length">Number of bytes to calculate the hash for</param>
  /// <param name="seed">Seed value to base the hash on</param>
  /// <returns>The murmur hash value of the specified byte sequence</returns>
  std::uint32_t calculateMurmur32(
    const std::uint8_t *data, std::size_t length, std::uint32_t seed
  ) {
    const std::uint32_t mixFactor = 0x5bd1e995;
    const int mixShift = 24;

    std::uint32_t hash = seed ^ static_cast<std::uint32_t>(length * mixFactor);

    while(length >= 4) {
      std::uint32_t data32 = *reinterpret_cast<const std::uint32_t *>(data);

      data32 *= mixFactor;
      data32 ^= data32 >> mixShift;
      data32 *= mixFactor;

      hash *= mixFactor;
      hash ^= data32;

      data += 4;
      length -= 4;
    }

    switch(length) {
      case 3: { hash ^= std::uint32_t(data[2]) << 16; [[fallthrough]]; }
      case 2: { hash ^= std::uint32_t(data[1]) << 8; [[fallthrough]]; }
      case 1: { hash ^= std::uint32_t(data[0]); }
    };

    hash ^= hash >> 13;
    hash *= mixFactor;
    hash ^= hash >> 15;

    return hash;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the 64 bit murmur hash of a byte sequence</summary>
  /// <param name="key">Data for which the murmur hash will be calculated</param>
  /// <param name="length">Number of bytes to calculate the hash for</param>
  /// <param name="seed">Seed value to base the hash on</param>
  /// <returns>The murmur hash value of the specified byte sequence</returns>
  std::uint64_t calculateMurmur64(
    const std::uint8_t *data, std::size_t length, std::uint64_t seed
  ) {
    const std::uint64_t mixFactor = 0xc6a4a7935bd1e995ULL;
    const int mixShift = 47;

    std::uint64_t hash = seed ^ static_cast<std::uint64_t>(length * mixFactor);

    // Process the data in 64 bit chunks until we're down to the last few bytes
    while(length >= 8) {
      std::uint64_t data64 = *reinterpret_cast<const std::uint64_t *>(data);

      data64 *= mixFactor;
      data64 ^= data64 >> mixShift;
      data64 *= mixFactor;

      hash ^= data64;
      hash *= mixFactor;

      data += 8;
      length -= 8;
    }

    // Process the remaining 7 or less bytes
    switch(length) {
      case 7: { hash ^= std::uint64_t(data[6]) << 48; [[fallthrough]]; }
      case 6: { hash ^= std::uint64_t(data[5]) << 40; [[fallthrough]]; }
      case 5: { hash ^= std::uint64_t(data[4]) << 32; [[fallthrough]]; }
      case 4: { hash ^= std::uint64_t(data[3]) << 24; [[fallthrough]]; }
      case 3: { hash ^= std::uint64_t(data[2]) << 16; [[fallthrough]]; }
      case 2: { hash ^= std::uint64_t(data[1]) << 8; [[fallthrough]]; }
      case 1: { hash ^= std::uint64_t(data[0]); hash *= mixFactor; }
    };

    // Also apply the bit mixing operation to the last few bytes
    hash ^= hash >> mixShift;
    hash *= mixFactor;
    hash ^= hash >> mixShift;

    return hash;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::size_t CaseInsensitiveUtf8Hash::operator()(const std::u8string &text) const noexcept {
    static const std::uint8_t aslrSeed = 0;
    std::size_t hash = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(&aslrSeed));

    using Nuclex::Support::Text::UnicodeHelper;

    const char8_t *current = text.c_str();
    const char8_t *end = current + text.length();
    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      requireValidCodePoint(codePoint);
      codePoint = UnicodeHelper::ToFoldedLowercase(codePoint);

      // We're abusing the Murmur hashing function a bit here. It's not intended for
      // incremental generation and this will likely decrease hashing quality...
      if constexpr(sizeof(std::size_t) >= 8) {
        hash = calculateMurmur64(
          reinterpret_cast<const std::uint8_t *>(&codePoint), 4,
          static_cast<std::uint32_t>(hash)
        );
      } else {
        hash = calculateMurmur32(
          reinterpret_cast<const std::uint8_t *>(&codePoint), 4,
          static_cast<std::uint32_t>(hash)
        );
      }
    }

    return hash;
  }

  // ------------------------------------------------------------------------------------------- //

  bool CaseInsensitiveUtf8EqualTo::operator()(
    const std::u8string &left, const std::u8string &right
  ) const noexcept {
    return StringMatcher::AreEqual<false>(left, right);
  }

  // ------------------------------------------------------------------------------------------- //

  bool CaseInsensitiveUtf8Less::operator()(
    const std::u8string &left, const std::u8string &right
  ) const noexcept {
    using Nuclex::Support::Text::UnicodeHelper;

    const char8_t *leftStart = left.c_str();
    const char8_t *leftEnd = leftStart + left.length();
    const char8_t *rightStart = right.c_str();
    const char8_t *rightEnd = rightStart + right.length();

    for(;;) {
      if(leftStart >= leftEnd) {
        if(rightStart >= rightEnd) {
          return false; // false because both equal up to here
        } else {
          return true; // true because left is shorter
        }
      }
      if(rightStart >= rightEnd) {
        return false; // false because left is longer
      }

      char32_t leftCodePoint = UnicodeHelper::ReadCodePoint(leftStart, leftEnd);
      requireValidCodePoint(leftCodePoint);
      char32_t rightCodePoint = UnicodeHelper::ReadCodePoint(rightStart, rightEnd);
      requireValidCodePoint(rightCodePoint);

      leftCodePoint = UnicodeHelper::ToFoldedLowercase(leftCodePoint);
      rightCodePoint = UnicodeHelper::ToFoldedLowercase(rightCodePoint);

      if(leftCodePoint < rightCodePoint) {
        return true;
      } else if(leftCodePoint > rightCodePoint) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
