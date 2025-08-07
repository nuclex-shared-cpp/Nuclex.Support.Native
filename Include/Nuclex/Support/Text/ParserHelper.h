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

#ifndef NUCLEX_SUPPORT_TEXT_PARSERHELPER_H
#define NUCLEX_SUPPORT_TEXT_PARSERHELPER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::u8string
#include <optional> // for std::optional
#include <cstdint> // for std::uint32_t, std::int32_t, std::uint64_t, std::int64_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods for parsing text-based file formats</summary>
  /// <remarks>
  ///   <para>
  ///     For generic character classification, also see the cctype header in C++ which
  ///     provides several methods to classify ASCII characters. Since all bytes in the ASCII
  ///     range remain unique in UTF-8 (all 2, 3 and 4 byte sequences have the highest bit
  ///     set), even if you feed each byte of an UTF-8 string to, say, ::isdigit(), it will
  ///     correctly identify all numbers.
  ///   </para>
  ///   <para>
  ///     The methods in this class offer alternatives for UTF-8 parsing. If the full UTF-8
  ///     range is required, the character is passed as a single UTF-32 unit (char32_t)
  ///     which can encode any unicode character in a fixed length (most UTF-8 libraries,
  ///     such as Nemanja Trifunovic's &quot;utfcpp&quot; library let you iterate through
  ///     an UTF-8 string by returning individual characters as UTF-32).
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ParserHelper {

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="asciiCharacter">
    ///   Single-byte ASCII character that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    /// <remarks>
    ///   This will obviously only cover whitespace variants in the ASCII range, but may
    ///   be sufficient if you're parsing a structured format such as XML, JSON or .ini
    ///   where either the specification limits the allowed whitespace variants outside of
    ///   strings/data or in cases where you're providing the input files yourself rather
    ///   than parsing data from the web or another application.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char asciiCharacter
    );

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="utf8Character">
    ///   Single UTF-8 code unit that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    /// <remarks>
    ///   This will obviously only cover whitespace variants in the ASCII range.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char8_t asciiCharacter
    );

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="codePoint">
    ///   Unicode code point that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char32_t codePoint
    );

    /// <summary>Checks if an UTF-8 string is either empty or contains only whitespace</summary>
    /// <param name="text">String that will be checked for being blank or empty</param>
    /// <returns>True if th string was empty or contained only whitespace</returns>
    public: NUCLEX_SUPPORT_API static bool IsBlankOrEmpty(const std::u8string &text);

    /// <summary>
    ///   Moves <paramref cref="start" /> ahead until the first non-whitespace UTF-8
    ///   character or until hitting <paramref cref="end" />
    /// </summary>
    /// <param name="start">Start pointer from which on whitespace will be skipped</param>
    /// <param name="end">End pointer that may not be overrun</param>
    public: NUCLEX_SUPPORT_API static void SkipWhitespace(
      const char8_t *&start, const char8_t *end
    );

    /// <summary>
    ///   Moves <paramref cref="start" /> ahead until the first whitespace UTF-8
    ///   character or until hitting <paramref cref="end" />
    /// </summary>
    /// <param name="start">Start pointer from which on non-whitespace will be skipped</param>
    /// <param name="end">End pointer that may not be overrun</param>
    public: NUCLEX_SUPPORT_API static void SkipNonWhitespace(
      const char8_t *&start, const char8_t *end
    );

    /// <summary>Searches for the next word (character surrounded by whitespace)</summary>
    /// <param name="start">
    ///   Start pointer from which on the word will be searched. Will be advanced to
    ///   the first character past the word
    /// </param>
    /// <param name="end">End pointer one past the last character to consider</param>
    /// <param name="word">
    ///   Optional pointer to a string_view that will be set to the entire next word
    /// </param>
    /// <remarks>
    ///   This method will look for a word in the input string. A word is considered to be
    ///   one or more non-whitespace characters, either surrounded by whitespace or bordering
    ///   the ends of the string. If the start index is on a word, that will be the word
    ///   extracted. Otherwise, the method will scan for the next word.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void FindWord(
      const char8_t *&start, const char8_t *end,
      std::u8string_view *word = nullptr
    );

    /// <summary>Searches for the next line break</summary>
    /// <param name="start">
    ///   Start pointer from which on the line will be searched. Will be advanced to
    ///   the first character past the discovered line break or to the end pointer
    /// </param>
    /// <param name="end">End pointer one past the last character to consider</param>
    /// <param name="line">
    ///   Optional pointer to a string_view that will be set to the entire line,
    ///   excluding any line break characters
    /// </param>
    /// <remarks>
    ///   <para>
    ///     This method will look for a line break in the input string. If the start index
    ///     is on a line break already, the start pointer will only be advanced past that
    ///     line break and an empty string will be stored in the <paramref cref="line" />
    ///     pointer, if provided. This behavior is needed in order to correctly report
    ///     empty lines back to the caller. If you're looking for non-empty lines, simply
    ///     call this method until finding a non-empty string or reaching the end pointer.
    ///   </para>
    ///   <para>
    ///     Note that this tries to deal with Windows-style line breaks (CR followed by LF),
    ///     but also accepts old Mac-style line breaks (just CR) and Linux/Unix-style line
    ///     breaks (just LF). This means that if you're chunking text and a chunk boundary
    ///     happens exactly between a CR and its LF, this method, unable to keep state
    ///     between the calls on the chunks, would report a spurious line break.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void FindLine(
      const char8_t *&start, const char8_t *end,
      std::u8string_view *line = nullptr
    );

#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)
    /// <summary>Attempts to parse the specified numeric type from the provided text</summary>
    /// <typeparam name="TScalar">
    ///   Type that will be parsed from the text. Must be either a 32 bit integer,
    ///   64 bit integer, float or double. Other types are not supported.
    /// </typeparam>
    /// <param name="start">
    ///   Pointer to the start of the textual data. Will be updated to the next byte
    ///   after the numeric type if parsing succeeds.
    /// </param>
    /// <param name="end">Byte at which the text ends</param>
    /// <returns>The parsed numeric type or an empty std::optional instance</returns>
    public: template<typename TScalar>
    inline static std::optional<TScalar> ParseNumber(
      const char8_t *&start, const char8_t *end
    );
#endif
  };

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(char asciiCharacter) {
    return (
      (
        (asciiCharacter >= char(0x09)) && // (see below)
        (asciiCharacter < char(0x0e))
      ) ||
      (asciiCharacter == char(0x20)) // space
    );
    // Covered via range:
    // (asciiCharacter == char(0x09)) || // tab
    // (asciiCharacter == char(0x0a)) || // line feed
    // (asciiCharacter == char(0x0b)) || // line tabulation
    // (asciiCharacter == char(0x0c)) || // form feed
    // (asciiCharacter == char(0x0d)) || // carriage return
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(char8_t utf8Character) {
    return (
      (
        (utf8Character >= char8_t(0x09)) && // (see below)
        (utf8Character < char8_t(0x0e))
      ) ||
      (utf8Character == char8_t(0x20)) // space
    );
    // Covered via range:
    // (utf8Character == char8_t(0x09)) || // tab
    // (utf8Character == char8_t(0x0a)) || // line feed
    // (utf8Character == char8_t(0x0b)) || // line tabulation
    // (utf8Character == char8_t(0x0c)) || // form feed
    // (utf8Character == char8_t(0x0d)) || // carriage return
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(char32_t unicodeCharacter) {
    switch(unicodeCharacter & char32_t(0xffffff00)) {
      case char32_t(0x0000): {
        return (
          (
            (unicodeCharacter >= char32_t(0x0009)) && // (see below)
            (unicodeCharacter < char32_t(0x000e))
          ) ||
          (unicodeCharacter == char32_t(0x0020)) || // space
          (unicodeCharacter == char32_t(0x0085)) || // next line
          (unicodeCharacter == char32_t(0x00A0))    // no-break space
        );
        // Covered via range:
        // (unicodeCharacter == char32_t(0x0009)) || // tab
        // (unicodeCharacter == char32_t(0x000a)) || // line feed
        // (unicodeCharacter == char32_t(0x000b)) || // line tabulation
        // (unicodeCharacter == char32_t(0x000c)) || // form feed
        // (unicodeCharacter == char32_t(0x000d)) || // carriage return
      }
      case char32_t(0x1600): {
        return (unicodeCharacter == char32_t(0x1680)); // ogham space mark
      }
      case char32_t(0x2000): {
        return (
          (unicodeCharacter < char32_t(0x200b)) || // (see below)
          (unicodeCharacter == char32_t(0x2028)) || // line separator
          (unicodeCharacter == char32_t(0x2029)) || // paragraph separator
          (unicodeCharacter == char32_t(0x202f)) || // narrow no-break space
          (unicodeCharacter == char32_t(0x205f))    // medium mathematical space
        );
        // Covered via range:
        // (utf8Character == char32_t(0x2000)) || // en quad
        // (utf8Character == char32_t(0x2001)) || // em quad
        // (utf8Character == char32_t(0x2002)) || // en space
        // (utf8Character == char32_t(0x2003)) || // em space
        // (utf8Character == char32_t(0x2004)) || // three-per-em space
        // (utf8Character == char32_t(0x2005)) || // four-per-em-space
        // (utf8Character == char32_t(0x2006)) || // six-per-em-space
        // (utf8Character == char32_t(0x2007)) || // figure space
        // (utf8Character == char32_t(0x2008)) || // punctuation space
        // (utf8Character == char32_t(0x2009)) || // thin space
        // (utf8Character == char32_t(0x200a))    // hair space
      }
      case char32_t(0x3000): {
        return (unicodeCharacter == char32_t(0x3000)); // ideographic space
      }
      default: {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  template<typename TScalar>
  inline std::optional<TScalar> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  ) {
    static_assert(
      std::is_same<TScalar, std::uint32_t>::value ||
      std::is_same<TScalar, std::int32_t>::value ||
      std::is_same<TScalar, std::uint64_t>::value ||
      std::is_same<TScalar, std::int64_t>::value ||
      std::is_same<TScalar, float>::value ||
      std::is_same<TScalar, double>::value,
      u8"Only 32/64 bit unsigned/signed integers, floats and doubles are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API std::optional<std::uint32_t> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::int32_t> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::uint64_t> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::int64_t> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<float> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<double> ParserHelper::ParseNumber(
    const char8_t *&start, const char8_t *end
  );
#endif // defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_PARSERHELPER_H
