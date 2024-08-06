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

#include "Nuclex/Support/Text/ParserHelper.h"
#include "Nuclex/Support/Text/UnicodeHelper.h"
#include "Nuclex/Support/Errors/CorruptStringError.h"

#include <cstdlib> // for std::strtoul(), std::strtoull(), std::strtol(), std::strtoll()

namespace {

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)
  /// <summary>Skips over an integer in textual form</summary>
  /// <param name="start">Pointer to the start of the text, will be updated</param>
  /// <param name="end">Pointer one past the last character of the text</param>
  /// <returns>
  ///   True if <paramref cref="start" /> has skipped a valid integer,
  ///   false if not valid integer was found (and <paramref cref="start" /> contains junk)
  /// </returns>
  bool skipInteger(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {

    // An optional plus or minus sign can follow
    if((*start == u8'+') || (*start == u8'-')) {
      ++start;
      if(start == end) { // ...but not only the plus/minus sign!
        return false;
      }
    }

    // If we now get a digit, we can already deliver an integer!
    if(std::isdigit(*start)) {
      ++start;
    }
    while(start != end) {
      if(!std::isdigit(*start)) {
        break;
      }
      ++start;
    }

    return true;

  }
#endif // defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw Nuclex::Support::Errors::CorruptStringError(
        u8"Illegal UTF-8 character(s) encountered"
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  bool ParserHelper::IsBlankOrEmpty(const std::string &text) {
    const Char8Type *current = reinterpret_cast<const Char8Type *>(text.c_str());
    const Char8Type *end = current + text.length();

    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      requireValidCodePoint(codePoint);

      if(!IsWhitespace(codePoint)) {
        return false;
      }
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void ParserHelper::SkipWhitespace(const Char8Type *&start, const Char8Type *end) {
    const Char8Type *current = start;
    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      requireValidCodePoint(codePoint);

      // We use this design to make 'start' lag one code point behind. This is needed
      // because ReadCodePoint() advances the read pointer, so our 'current' is already
      // past the first whitespace when we check the code point.
      if(IsWhitespace(codePoint)) {
        start = current;
      } else {
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ParserHelper::SkipNonWhitespace(const Char8Type *&start, const Char8Type *end) {
    const Char8Type *current = start;
    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      requireValidCodePoint(codePoint);

      // We use this design to make 'start' lag one code point behind. This is needed
      // because ReadCodePoint() advances the read pointer, so our 'current' is already
      // past the first whitespace when we check the code point.
      if(IsWhitespace(codePoint)) {
        break;
      } else {
        start = current;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ParserHelper::FindWord(
    const Char8Type *&start, const Char8Type *end,
    std::string_view *word /* = nullptr */
  ) {
    SkipWhitespace(start, end);

    // If the caller was interested in obtaining the word, scan for its end
    if(word != nullptr) {
      const Char8Type *current = start;
      SkipNonWhitespace(current, end);
      if(start < current) {
        *word = std::string_view(
          reinterpret_cast<const std::string_view::value_type *>(start),
          static_cast<std::string_view::size_type>(current - start)
        );
      } else {
        *word = std::string_view(); // to  ensure word.empty() returns true
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
