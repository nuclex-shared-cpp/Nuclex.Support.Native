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

#include "Nuclex/Support/Text/StringMatcher.h"
#include "Nuclex/Support/Text/UnicodeHelper.h"

#include "Utf8/checked.h" // remove this
#include "Utf8/unchecked.h" // remove this

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if 0 // This is a memento for my old, trusty Ascii wildcard checker. It's retired now.
  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  bool matchWildcardAscii(const char *text, const char *wildcard) {
    if(!text) {
      return false;
    }

    for(; '*' ^ *wildcard; ++wildcard, ++text) {
      if(!*text) {
        return (!*wildcard);
      }

      if(::toupper(*text) ^ ::toupper(*wildcard) && '?' ^ *wildcard) {
        return false;
      }
    }

    while('*' == wildcard[1]) {
      wildcard++;
    }

    do {
      if(matchWildcardAscii(text, wildcard + 1)) {
        return true;
      }
    } while(*text++);

    return false;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invidual UTF-8 character type (until C++20 introduces char8_t)</summary>
  typedef unsigned char my_char8_t;

  // ------------------------------------------------------------------------------------------- //

  // Helper until I've completely removed the utf8cpp library from this file
  std::uint32_t toFoldedLowercase(std::uint32_t codePoint) {
    return static_cast<std::uint32_t>(
      Nuclex::Support::Text::UnicodeHelper::ToFoldedLowercase(static_cast<char32_t>(codePoint))
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw std::invalid_argument(u8"Illegal UTF-8 character(s) encountered");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <typeparam name="CaseSensitive">Whether the comparison will be case-sensitive</typeparam>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  template<bool CaseSensitive>
  bool matchWildcardUtf8(
    const my_char8_t *text, const my_char8_t *textEnd,
    const my_char8_t *wildcard, const my_char8_t *wildcardEnd
  ) {
    using Nuclex::Support::Text::UnicodeHelper;
    assert((text != nullptr) && u8"Text must not be a NULL pointer");
    assert((wildcard != nullptr) && u8"Wildcard must not be a NULL pointer");

    char32_t wildcardCodePoint;
    for(;;) {

      // If the end of the wildcard is reached, all letters of the input text
      // must have been consumed (unless the wildcard ends with a star)
      if(wildcard >= wildcardEnd) {
        return (text >= textEnd); // All letters must have been consumed
      }

      // Try to obtain the next wild card letter. We do this before checking
      // for the end of the text because wildcards can match zero letters, too.
      char32_t wildcardCodePoint = UnicodeHelper::ReadCodePoint(wildcard, wildcardEnd);
      requireValidCodePoint(wildcardCodePoint);
      if(wildcardCodePoint == U'*') {
        break; // Wildcard had a star, enter skip mode
      }

      // If text ends but wildcard has more letters to match
      if(text >= textEnd) {
        return false;
      }

      // We have both a valid wildcard letter and a letter from the text to compare
      // against it, so lets compare one input letter against one wildcard letter
      char32_t textCodePoint = UnicodeHelper::ReadCodePoint(text, textEnd);
      requireValidCodePoint(textCodePoint);
      if(wildcardCodePoint != U'?') {
        if constexpr(CaseSensitive) {
          if(textCodePoint != wildcardCodePoint) {
            return false;
          }
        } else { // Case insensitive
          textCodePoint = UnicodeHelper::ToFoldedLowercase(textCodePoint);
          if(textCodePoint != UnicodeHelper::ToFoldedLowercase(wildcardCodePoint)) {
            return false;
          }
        }
      }

    } // letterwise comparison loop

    // If we encountered a star, first skip any redundant stars directly following
    const my_char8_t *wildcardAfterStar = wildcard;
    for(;;) {
      if(wildcard >= wildcardEnd) {
        return true; // If the wildcard ends with a star, any remaining text is okay!
      }

      // Read the next letter from the wildcard and see if it's a star, too
      wildcardCodePoint = UnicodeHelper::ReadCodePoint(wildcard, wildcardEnd);
      requireValidCodePoint(wildcardCodePoint);
      if(wildcardCodePoint != U'*') {
        break;
      }

      // Wildcard letter was indeed a star, so the current 'wildcard' pointer
      // (already advanced to the next letter) is the earliest possible non-star
      wildcardAfterStar = wildcard;
    }

    // Then retry the wildcard match skipping any number of characters from text
    // (the star can match anything from zero to all characters)
    while(text < textEnd) {
      if(matchWildcardUtf8<CaseSensitive>(text, textEnd, wildcardAfterStar, wildcardEnd)) {
        return true;
      }

      // This could just skip, but if we encounter an invalid character,
      // we can't be sure of its length, so have to check and bail out in case.
      char32_t textCodePoint = UnicodeHelper::ReadCodePoint(text, textEnd);
      requireValidCodePoint(textCodePoint);
    }

    // No amount of skipping helped, there's no match
    return false;

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string contains another string</summary>
  /// <param name="haystack">Text that will be scanned for the other string</param>
  /// <param name="needle">Substring that will be searched for</param>
  /// <returns>
  ///   The address in the haystack string where the first match was found or a null pointer
  ///   if no matches were found
  /// </returns>
  const char *findSubstringUtf8(const char *haystack, const char *needle) {
    std::uint32_t firstNeedleCodepoint = utf8::unchecked::next(needle);
    if(firstNeedleCodepoint == 0) {
      return haystack;
    }
    firstNeedleCodepoint = toFoldedLowercase(firstNeedleCodepoint);
    const char *needleFromSecondCodepoint = needle;

    const char *haystackAtStart = haystack;
    for(;;) {
      std::uint32_t haystackCodepoint = utf8::unchecked::next(haystack);

      // Did the run out of hay without finding the needle? Then there is no match.
      if(unlikely(haystackCodepoint == 0)) {
        return nullptr;
      }

      // In the outer loop, scan only for the a match of the first needle codepoint.
      // Keeping this loop tight allows the compiler to optimize it into a simple scan.
      if(unlikely(toFoldedLowercase(haystackCodepoint) == firstNeedleCodepoint)) {
        std::uint32_t needleCodepoint = firstNeedleCodepoint;
        const char *current = haystack;
        do {
          needleCodepoint = utf8::unchecked::next(needle);
          if(needleCodepoint == 0) { // Match is complete when needle ends!
            return haystackAtStart;
          }

          haystackCodepoint = utf8::unchecked::next(current);
          if(haystackCodepoint == 0) {
            break;
          }
        } while(toFoldedLowercase(haystackCodepoint) == toFoldedLowercase(needleCodepoint));

        // No match found. Reset the needle for the next scan.
        needle = needleFromSecondCodepoint;
      }

      // Since no match was found, update the start pointer so we still have
      // a pointer to the starting position when the needle starts matching.
      haystackAtStart = haystack;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string contains another string</summary>
  /// <param name="haystack">Text that will be scanned for the other string</param>
  /// <param name="needle">Substring that will be searched for</param>
  /// <returns>
  ///   The address in the haystack string where the first match was found or a null pointer
  ///   if no matches were found
  /// </returns>
  const char *findSubstringUtf8CaseSensitive(const char *haystack, const char *needle) {
    std::uint32_t firstNeedleCodepoint = utf8::unchecked::next(needle);
    if(firstNeedleCodepoint == 0) {
      return haystack;
    }
    const char *needleFromSecondCodepoint = needle;

    const char *haystackAtStart = haystack;
    for(;;) {
      std::uint32_t haystackCodepoint = utf8::unchecked::next(haystack);

      // Did the run out of hay without finding the needle? Then there is no match.
      if(unlikely(haystackCodepoint == 0)) {
        return nullptr;
      }

      // In the outer loop, scan only for the a match of the first needle codepoint.
      // Keeping this loop tight allows the compiler to optimize it into a simple scan.
      if(unlikely(haystackCodepoint == firstNeedleCodepoint)) {
        std::uint32_t needleCodepoint = firstNeedleCodepoint;
        const char *current = haystack;
        do {
          needleCodepoint = utf8::unchecked::next(needle);
          if(needleCodepoint == 0) { // Match is complete when needle ends!
            return haystackAtStart;
          }

          haystackCodepoint = utf8::unchecked::next(current);
          if(haystackCodepoint == 0) {
            break;
          }
        } while(haystackCodepoint == needleCodepoint);

        // No match found. Reset the needle for the next scan.
        needle = needleFromSecondCodepoint;
      }

      // Since no match was found, update the start pointer so we still have
      // a pointer to the starting position when the needle starts matching.
      haystackAtStart = haystack;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string starts with another string</summary>
  /// <param name="haystack">Text whose beginning will be checked for the other string</param>
  /// <param name="needle">Substring that will be searched for</param>
  /// <returns>True if the 'haystack' string starts with the 'needle' string</returns>
  bool checkStringStartsWithUtf8(const char *haystack, const char *needle) {
    for(;;) {
      std::uint32_t needleCodepoint = utf8::unchecked::next(needle);
      if(needleCodepoint == 0) {
        return true;
      }

      std::uint32_t haystackCodepoint = utf8::unchecked::next(haystack);
      if(haystackCodepoint == 0) {
        return false;
      }

      if(toFoldedLowercase(needleCodepoint) != toFoldedLowercase(haystackCodepoint)) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string starts with another string</summary>
  /// <param name="haystack">Text whose beginning will be checked for the other string</param>
  /// <param name="needle">Substring that will be searched for</param>
  /// <returns>True if the 'haystack' string starts with the 'needle' string</returns>
  bool checkStringStartsWithUtf8CaseSensitive(const char *haystack, const char *needle) {
    for(;;) {
      std::uint32_t needleCodepoint = utf8::unchecked::next(needle);
      if(needleCodepoint == 0) {
        return true;
      }

      std::uint32_t haystackCodepoint = utf8::unchecked::next(haystack);
      if(haystackCodepoint == 0) {
        return false;
      }

      if(needleCodepoint != haystackCodepoint) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the 32 bit murmur hash of a byte sequence</summary>
  /// <param name="key">Data for which the murmur hash will be calculated</param>
  /// <param name="length">Number of bytes to calculate the hash for</param>
  /// <param name="seed">Seed value to base the hash on</param>
  /// <returns>The murmur hash value of the specified byte sequence</returns>
  std::uint32_t CalculateMurmur32(
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
  std::uint64_t CalculateMurmur64(
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

  bool StringMatcher::AreEqual(
    const std::string &left, const std::string &right, bool caseSensitive /* = false */
  ) {
    if(caseSensitive) {
      return (left == right);
    } else {
      if(left.length() != right.length()) {
        return false;
      }

      const my_char8_t *currentLeft = reinterpret_cast<const my_char8_t *>(left.c_str());
      const my_char8_t *leftEnd = currentLeft + left.length();

      const my_char8_t *currentRight = reinterpret_cast<const my_char8_t *>(right.c_str());
      const my_char8_t *rightEnd = currentRight + right.length();

      for(;;) {
        if(currentLeft >= leftEnd) {
          return (currentRight >= rightEnd); // Both must end at the same time
        } else if(currentRight >= rightEnd) {
          return false; // right ended before left
        }

        char32_t leftCodePoint = UnicodeHelper::ReadCodePoint(currentLeft, leftEnd);
        char32_t rightCodePoint = UnicodeHelper::ReadCodePoint(currentRight, rightEnd);

        leftCodePoint = UnicodeHelper::ToFoldedLowercase(leftCodePoint);
        rightCodePoint = UnicodeHelper::ToFoldedLowercase(rightCodePoint);
        if(leftCodePoint != rightCodePoint) {
          return false;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StringMatcher::Contains(
    const std::string &haystack, const std::string &needle, bool caseSensitive /* = false */
  ) {
    if(caseSensitive) {
      return findSubstringUtf8CaseSensitive(haystack.c_str(), needle.c_str()) != nullptr;
    } else {
      return findSubstringUtf8(haystack.c_str(), needle.c_str()) != nullptr;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StringMatcher::StartsWith(
    const std::string &haystack, const std::string &needle, bool caseSensitive /* = false */
  ) {
    if(caseSensitive) {
      return checkStringStartsWithUtf8CaseSensitive(haystack.c_str(), needle.c_str());
    } else {
      //return (haystack.rfind(needle, 0) == 0);
      return checkStringStartsWithUtf8(haystack.c_str(), needle.c_str());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StringMatcher::FitsWildcard(
    const std::string &text, const std::string &wildcard, bool caseSensitive /* = false */
  ) {
    const my_char8_t *textStart = reinterpret_cast<const my_char8_t *>(text.c_str());
    const my_char8_t *textEnd = textStart + text.length();

    const my_char8_t *wildcardStart = reinterpret_cast<const my_char8_t *>(wildcard.c_str());
    const my_char8_t *wildcardEnd = wildcardStart + wildcard.length();

    if(caseSensitive) {
      return matchWildcardUtf8<true>(textStart, textEnd, wildcardStart, wildcardEnd);
    } else {
      return matchWildcardUtf8<false>(textStart, textEnd, wildcardStart, wildcardEnd);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t CaseInsensitiveUtf8Hash::operator()(const std::string &text) const noexcept {
    static const std::uint8_t aslrSeed = 0;
    std::size_t hash = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(&aslrSeed));

    const char *characters = text.c_str();
    for(;;) {
      std::uint32_t codepoint = utf8::unchecked::next(characters);
      if(codepoint == 0) {
        return hash;
      }

      codepoint = toFoldedLowercase(codepoint);

      // We're abusing the Murmur hashing function a bit here. It's not intended for
      // incremental generation and this will likely decrease hashing quality...
      if constexpr(sizeof(std::size_t) >= 8) {
        hash = CalculateMurmur64(
          reinterpret_cast<const std::uint8_t *>(&codepoint), 4,
          static_cast<std::uint32_t>(hash)
        );
      } else {
        hash = CalculateMurmur32(
          reinterpret_cast<const std::uint8_t *>(&codepoint), 4,
          static_cast<std::uint32_t>(hash)
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool CaseInsensitiveUtf8EqualTo::operator()(
    const std::string &left, const std::string &right
  ) const noexcept {
    const char *leftCharacters = left.c_str();
    const char *rightCharacters = right.c_str();
    for(;;) {
      std::uint32_t leftCodepoint = utf8::unchecked::next(leftCharacters);
      std::uint32_t rightCodepoint = utf8::unchecked::next(rightCharacters);
      if(leftCodepoint == 0) {
        return (rightCodepoint == 0);
      } else if(rightCodepoint == 0) {
        return false;
      }

      if(toFoldedLowercase(leftCodepoint) != toFoldedLowercase(rightCodepoint)) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool CaseInsensitiveUtf8Less::operator()(
    const std::string &left, const std::string &right
  ) const noexcept {
    const char *leftCharacters = left.c_str();
    const char *rightCharacters = right.c_str();
    for(;;) {
      std::uint32_t leftCodepoint = utf8::unchecked::next(leftCharacters);
      std::uint32_t rightCodepoint = utf8::unchecked::next(rightCharacters);
      if(leftCodepoint == 0) {
        return (rightCodepoint != 0); // true if left is shorter
      } else if(rightCodepoint == 0) {
        return false; // false because left is longer
      }

      leftCodepoint = toFoldedLowercase(leftCodepoint);
      rightCodepoint = toFoldedLowercase(rightCodepoint);
      if(leftCodepoint < rightCodepoint) {
        return true;
      } else if(leftCodepoint > rightCodepoint) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
