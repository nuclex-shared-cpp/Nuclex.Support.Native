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

#include "Nuclex/Support/Text/StringHelper.h"

#include "Nuclex/Support/Text/UnicodeHelper.h"
#include "Nuclex/Support/Text/ParserHelper.h"
#include "Nuclex/Support/Errors/CorruptStringError.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Collapses any instance of two or more consecutive whitespaces into a single whitespace
  /// </summary>
  /// <typeparam name="StringType">Type of string the method will be working on</typeparam>
  /// <typeparam name="CharType">
  ///   Type of the UTF characters in the string, must be char8_t, char16_t or char32_t
  /// </typeparam>
  /// <param name="targetString">String in which whitespace will be collapsed</param>
  template<typename StringType, typename CharType>
  void collapseDuplicateWhitespaceAndTrim(StringType &targetString) {
    using Nuclex::Support::Text::UnicodeHelper;
    using Nuclex::Support::Text::ParserHelper;

    CharType *read = reinterpret_cast<CharType *>(targetString.data());
    CharType *write = read; // 'write' tracks the shift target position
    const CharType *end = read + targetString.length();

    // If the string is of zero length, we don't need to do anything
    if(unlikely(read == end)) {
      return;
    }

    // Read the first character. This variant does trimming, so the first character
    // decides if we can even run the scan-only loop (and doing the check outside of
    // the loop simplifies the conditions that need to be checked inside the loop)
    char32_t codePoint = UnicodeHelper::ReadCodePoint(read, end);
    if(unlikely(codePoint == char32_t(-1))) {
      throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
    }

    // If it was not a whitespace, we can fast-forward until we find a duplicate whitespace
    if(ParserHelper::IsWhitespace(codePoint)) {
      for(;;) {
        if(unlikely(read >= end)) {
          targetString.resize(0); // Only whitespace + trim = string becomes empty
          return;
        }

        codePoint = UnicodeHelper::ReadCodePoint(read, end);
        if(unlikely(codePoint == char32_t(-1))) {
          throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
        }

        if(likely(!ParserHelper::IsWhitespace(codePoint))) {
          break; // Exit without updating write pointer since we're trimming
        }
      } // for ever
    } else { // initial character is not a whitespace
      write = read;
      std::size_t successiveWhitespaceCount = 0;
      for(;;) {
        if(unlikely(read >= end)) {
          targetString.resize(write - reinterpret_cast<CharType *>(targetString.data()));
          return;
        }

        codePoint = UnicodeHelper::ReadCodePoint(read, end);
        if(unlikely(codePoint == char32_t(-1))) {
          throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
        }

        if(unlikely(ParserHelper::IsWhitespace(codePoint))) {
          ++successiveWhitespaceCount;
        } else if(unlikely(successiveWhitespaceCount >= 2)) { // String will need backshifting
          UnicodeHelper::WriteCodePoint(write, U' ');
          break;
        } else { // Character after single whitespace (which we'll just skip over)
          write = read; // write pointer tracks last non-whitespace
          successiveWhitespaceCount = 0;
        }
      } // for ever
    } // if initial character whitespace / not whitespace

    // At this point:
    // - 'read' is on a non-whitespace character
    // - 'write' is at the current backshifting target position
    // - 'codePoint' contains one code point that yet needs to be backshifted
    UnicodeHelper::WriteCodePoint(write, codePoint);

    // Backshifting loop
    {
      std::size_t successiveWhitespaceCount = 0;
      char32_t whitespaceCodePoint = codePoint;
      while(likely(read < end)) {
        codePoint = UnicodeHelper::ReadCodePoint(read, end);
        if(unlikely(codePoint == char32_t(-1))) {
          throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
        }

        if(unlikely(ParserHelper::IsWhitespace(codePoint))) {
          whitespaceCodePoint = codePoint;
          ++successiveWhitespaceCount;
        } else {
          if(unlikely(successiveWhitespaceCount >= 2)) { // Normalize multiple whitespaces into one
            UnicodeHelper::WriteCodePoint(write, U' ');
          } else if(unlikely(successiveWhitespaceCount == 1)) { // Pass through single whitespace
            UnicodeHelper::WriteCodePoint(write, whitespaceCodePoint);
          }
          UnicodeHelper::WriteCodePoint(write, codePoint);
          successiveWhitespaceCount = 0;
        }
      } // while read characters remain

      targetString.resize(write - reinterpret_cast<CharType *>(targetString.data()));
    } // beauty scope
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Collapses any instance of two or more consecutive whitespaces into a single whitespace
  /// </summary>
  /// <typeparam name="StringType">Type of string the method will be working on</typeparam>
  /// <typeparam name="CharType">
  ///   Type of the UTF characters in the string, must be char8_t, char16_t or char32_t
  /// </typeparam>
  /// <param name="targetString">String in which whitespace will be collapsed</param>
  template<typename StringType, typename CharType>
  void collapseDuplicateWhitespaceWithoutTrim(StringType &targetString) {
    using Nuclex::Support::Text::UnicodeHelper;
    using Nuclex::Support::Text::ParserHelper;

    CharType *read = reinterpret_cast<CharType *>(targetString.data());
    CharType *write = read; // 'write' tracks the shift target position
    const CharType *end = read + targetString.length();

    std::size_t successiveWhitespaceCount = 0;
    char32_t codePoint;

    for(;;) {
      if(unlikely(read >= end)) {
        if(unlikely(successiveWhitespaceCount >= 2)) {
          UnicodeHelper::WriteCodePoint(write, U' ');
          targetString.resize(write - reinterpret_cast<CharType *>(targetString.data()));
        } // Otherwise, even if final character was single whitespace, string is fine.

        return;
      }

      codePoint = UnicodeHelper::ReadCodePoint(read, end);
      if(unlikely(codePoint == char32_t(-1))) {
        throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
      }

      if(unlikely(ParserHelper::IsWhitespace(codePoint))) {
        ++successiveWhitespaceCount;
      } else if(unlikely(successiveWhitespaceCount >= 2)) {
        UnicodeHelper::WriteCodePoint(write, U' ');
        successiveWhitespaceCount = 0;
        break; // From here on out, we need to backshift the string
      } else {
        write = read; // Write pointer keeps tracking last non-whitespace character
        successiveWhitespaceCount = 0;
      }
    } // for ever
    
    // At this point:
    // - 'read' is on a non-whitespace character
    // - 'write' is at the current backshifting target position
    // - 'codePoint' contains one code point that yet needs to be backshifted
    UnicodeHelper::WriteCodePoint(write, codePoint);

    // Backshifting loop
    {
      char32_t whitespaceCodePoint = codePoint;
      while(likely(read < end)) {
        codePoint = UnicodeHelper::ReadCodePoint(read, end);
        if(unlikely(codePoint == char32_t(-1))) {
          throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
        }

        if(unlikely(ParserHelper::IsWhitespace(codePoint))) {
          whitespaceCodePoint = codePoint;
          ++successiveWhitespaceCount;
        } else {
          if(unlikely(successiveWhitespaceCount >= 2)) { // Normalize multiple whitespaces
            UnicodeHelper::WriteCodePoint(write, U' ');
          } else if(unlikely(successiveWhitespaceCount == 1)) { // Pass through single whitespace
            UnicodeHelper::WriteCodePoint(write, whitespaceCodePoint);
          }
          UnicodeHelper::WriteCodePoint(write, codePoint);
          successiveWhitespaceCount = 0;
        }
      } // while read characters remain

      if(unlikely(successiveWhitespaceCount >= 2)) { // Normalize multiple whitespaces into one
        UnicodeHelper::WriteCodePoint(write, U' ');
      } else if(unlikely(successiveWhitespaceCount == 1)) { // Pass through single whitespace
        UnicodeHelper::WriteCodePoint(write, whitespaceCodePoint);
      }

      targetString.resize(write - reinterpret_cast<CharType *>(targetString.data()));
    } // beauty scope
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Erases all first-level occurrences of the specified victim string</summary>
  /// <typeparam name="StringType">Type of string the method will be working on</typeparam>
  /// <typeparam name="CharType">
  ///   Type of the UTF characters in the string, must be char8_t, char16_t or char32_t
  /// </typeparam>
  /// <param name="targetString">String in which victims will be erased</param>
  /// <param name="victim">String that will be erased from the target string</param>
  template<typename StringType, typename CharType>
  void eraseSubstrings(StringType &targetString, const StringType &victim) {
    using Nuclex::Support::Text::UnicodeHelper;
    using Nuclex::Support::Text::ParserHelper;

    // Gather some pointers for moving around in the substring for comparison
    const CharType *victimFromSecondCodePoint = (
      reinterpret_cast<const CharType *>(victim.c_str())
    );
    const CharType *victimEnd = (
      victimFromSecondCodePoint + victim.length()
    );
    if(victimFromSecondCodePoint >= victimEnd) {
      return; // victim is empty, we were asked to remove nothing, so we do nothing
    }
    char32_t firstCodePointOfVictim = UnicodeHelper::ReadCodePoint(
      victimFromSecondCodePoint, victimEnd
    );
    if(unlikely(firstCodePointOfVictim == char32_t(-1))) {
      throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
    }

    // CHECK: Should we optimize this to stop comparison when master < substring?
    //   If there aren't enough characters left to fit the substring even once,
    //   it cannot occur any more. But our expected typical use case is removal of
    //   short tokens, so the additional check might even make things slower overall

    // We also need pointers into the master string so we can scan it for
    // occurrences of the substring and move characters to the left after removal.
    CharType *read = reinterpret_cast<CharType *>(targetString.data());
    CharType *write = read;
    const CharType *end = read + targetString.length();
    while(likely(read < end)) {
      char32_t currentCodePoint = UnicodeHelper::ReadCodePoint(read, end);
      if(unlikely(currentCodePoint == char32_t(-1))) {
        throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
      }

      // Once we encounter a character that matches the first character of the substring,
      // start comparing the rest of the substring to see if we have a match.
      if(unlikely(currentCodePoint == firstCodePointOfVictim)) {
        CharType *readForComparison = read;
        const CharType *victimCurrent = victimFromSecondCodePoint;
        while(likely(victimCurrent < victimEnd)) {
          if(readForComparison >= end) {
            break; // master string ended before full substring was compared
          }

          char32_t masterCodePoint = UnicodeHelper::ReadCodePoint(readForComparison, end);
          if(unlikely(masterCodePoint == char32_t(-1))) {
            throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
          }
          char32_t victimCodePoint = UnicodeHelper::ReadCodePoint(
            victimCurrent, victimEnd
          );
          if(unlikely(victimCodePoint == char32_t(-1))) {
            throw Nuclex::Support::Errors::CorruptStringError(u8"Corrupt UTF-8 string");
          }
          if(masterCodePoint != victimCodePoint) {
            break; // we found a difference, it doesn't match the full substring
          }
        }

        // If the full substring was matched
        if(victimCurrent == victimEnd) {
          read = readForComparison; // Skip over the substring
        } else { // Substring not matched, write character as normal
          UnicodeHelper::WriteCodePoint(write, currentCodePoint);
        }
      } else { // Character was not the start of the substring, write it as normal
        UnicodeHelper::WriteCodePoint(write, currentCodePoint);
      }
    }

    // Since the above loop keeps going until the end of the master string is reached,
    // in case substrings were found and skipped, it will already have moved all
    // of the remaining characters to the left, so the string contents are all in place.

    // We merely may need to tell the master string its new length in case it changed.
    if(read != write) {
      targetString.resize(
        write - reinterpret_cast<CharType *>(targetString.data())
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::CollapseDuplicateWhitespace(
    std::string &utf8String, bool alsoTrim /* = true */
  ) {
    if(alsoTrim) {
      collapseDuplicateWhitespaceAndTrim<std::string, UnicodeHelper::Char8Type>(utf8String);
    } else {
      collapseDuplicateWhitespaceWithoutTrim<std::string, UnicodeHelper::Char8Type>(utf8String);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::CollapseDuplicateWhitespace(
    std::wstring &wideString, bool alsoTrim /* = true */
  ) {
    if(alsoTrim) {
      if constexpr(sizeof(std::wstring::value_type) == sizeof(char32_t)) {
        collapseDuplicateWhitespaceAndTrim<std::wstring, char32_t>(wideString);
      } else {
        collapseDuplicateWhitespaceAndTrim<std::wstring, char16_t>(wideString);
      }
    } else {
      if constexpr(sizeof(std::wstring::value_type) == sizeof(char32_t)) {
        collapseDuplicateWhitespaceWithoutTrim<std::wstring, char32_t>(wideString);
      } else {
        collapseDuplicateWhitespaceWithoutTrim<std::wstring, char16_t>(wideString);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::EraseSubstrings(
    std::string &utf8String, const std::string &victim
  ) {
    eraseSubstrings<std::string, UnicodeHelper::Char8Type>(utf8String, victim);
  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::EraseSubstrings(
    std::wstring &wideString, const std::wstring &victim
  ) {
    if constexpr(sizeof(std::wstring::value_type) == sizeof(char32_t)) {
      eraseSubstrings<std::wstring, char32_t>(wideString, victim);
    } else {
      eraseSubstrings<std::wstring, char16_t>(wideString, victim);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
