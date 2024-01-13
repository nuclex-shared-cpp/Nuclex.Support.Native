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

    if(previousWasWhitespace && !alsoTrim) {
      *write = u8' ';
      ++write;
    }

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

  }

  // ------------------------------------------------------------------------------------------- //

  void StringHelper::EraseSubstrings(
    std::wstring &wideString, const std::wstring &victim
  ) {

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
