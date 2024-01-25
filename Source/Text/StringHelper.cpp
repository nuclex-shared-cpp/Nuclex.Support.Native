#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "Nuclex/Support/Text/StringHelper.h"

#include "Nuclex/Support/Text/UnicodeHelper.h"
#include "Nuclex/Support/Text/ParserHelper.h"

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::CollapseDuplicateWhitespace(
    std::string &utf8String, bool alsoTrim /* = true */
  ) {
    UnicodeHelper::char8_t *read = reinterpret_cast<UnicodeHelper::char8_t *>(
      utf8String.data()
    );
    UnicodeHelper::char8_t *end = read + utf8String.length();
    UnicodeHelper::char8_t *write = read;

    // Step through the string, one code point after another, to scan for whitespace.
    bool previousWasWhitespace = alsoTrim;
    while(read < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(read, end);

      if(ParserHelper::IsWhitespace(codePoint)) {
        if(!previousWasWhitespace) {
          *write = u8' ';
          ++write;
          previousWasWhitespace = true;

          // Conditions need to be better checked. 
          //break; // From this point on, we need to re-write the string to move it left
        }
      } else {
        UnicodeHelper::WriteCodePoint(codePoint, write);
        previousWasWhitespace = false;
      }
    }

    /*
    // Step through the string, one code point after another, to scan for whitespace,
    // but also move the string contents to the left since we removed at least one
    while(read < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(read, end);

      if(ParserHelper::IsWhitespace(codePoint)) {
        if(!previousWasWhitespace) {
          *write = u8' ';
          ++write;
          previousWasWhitespace = true;
        }
      } else {
        UnicodeHelper::WriteCodePoint(codePoint, write);
        previousWasWhitespace = false;
      }
    }
    */

    if(!previousWasWhitespace || alsoTrim) {
      --write;
    }
/*
    if(previousWasWhitespace && !alsoTrim) {
      *write = u8' ';
      ++write;
    }
*/
    --write;

    utf8String.resize(
      write - reinterpret_cast<UnicodeHelper::char8_t *>(utf8String.data())
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::CollapseDuplicateWhitespace(
    std::wstring &wideString, bool alsoTrim /* = true */
  ) {

  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::EraseSubstrings(
    std::string &utf8String, const std::string &victim
  ) {

    // Gather some pointers for moving around in the substring for comparison
    const UnicodeHelper::char8_t *substringFromSecondCharacter = (
      reinterpret_cast<const UnicodeHelper::char8_t *>(victim.c_str())
    );
    const UnicodeHelper::char8_t *substringEnd = (
      substringFromSecondCharacter + victim.length()
    );
    if(substringFromSecondCharacter >= substringEnd) {
      return; // substring is empty, we were asked to remove nothing, so we do nothing
    }
    char32_t firstCharacterOfSubstring = UnicodeHelper::ReadCodePoint(
      substringFromSecondCharacter, substringEnd
    );

    // CHECK: Should we optimize this to stop comparison when master < substring?
    //   If there aren't enough characters left to fit the substring even once,
    //   it cannot occur any more. But our expected typical use case is removal of
    //   short tokens, so the additional check might even make things slower overall

    // We also need pointers into the master string so we can scan it for
    // occurrences of the substring and move characters to the left after removal.
    UnicodeHelper::char8_t *read = (
      reinterpret_cast<UnicodeHelper::char8_t *>(utf8String.data())
    );
    UnicodeHelper::char8_t *write = read;
    const UnicodeHelper::char8_t *end = read + utf8String.length();
    while(read < end) {
      char32_t currentCharacter = UnicodeHelper::ReadCodePoint(read, end);

      // Once we encounter a character that matches the first character of the substring,
      // start comparing the rest of the substring to see if we have a match.
      if(currentCharacter == firstCharacterOfSubstring) {
        UnicodeHelper::char8_t *readForComparison = read;
        const UnicodeHelper::char8_t *substringCurrent = substringFromSecondCharacter;
        while(substringCurrent < substringEnd) {
          if(readForComparison >= end) {
            break; // master string ended before full substring was compared
          }

          char32_t masterCharacter = UnicodeHelper::ReadCodePoint(readForComparison, end);
          char32_t substringCharacter = UnicodeHelper::ReadCodePoint(
            substringCurrent, substringEnd
          );
          if(masterCharacter != substringCharacter) {
            break; // we found a difference, it doesn't match the full substring
          }
        }

        // If the full substring was matched
        if(substringCurrent == substringEnd) {
          read = readForComparison; // Skip over the substring
        } else { // Substring not matched, write character as normal
          UnicodeHelper::WriteCodePoint(currentCharacter, write);
        }
      } else { // Character was not the start of the substring, write it as normal
        UnicodeHelper::WriteCodePoint(currentCharacter, write);
      }
    }

    // Since the above loop keeps going until the end of the master string is reached,
    // in case substrings were found and skipped, it will already have moved all
    // of the remaining characters to the left, so the string contents are all in place.

    // We merely may need to tell the master string its new length in case it changed.
    if(read != write) {
      utf8String.resize(
        write - reinterpret_cast<UnicodeHelper::char8_t *>(utf8String.data())
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::EraseSubstrings(
    std::wstring &wideString, const std::wstring &victim
  ) {

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
