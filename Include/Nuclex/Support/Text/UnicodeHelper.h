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

#ifndef NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H
#define NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper methods for dealing with unicode and its different encodings</summary>
  /// <remarks>
  ///   <para>
  ///     Short overview of unicode: the &quot;unicode consortium&quot; has taken symbols from
  ///     all languages of the world and put them into a giant table. Said table is defined
  ///     with room for about 1.1 million symbols, but only some 140,000 symbols have been
  ///     filled so far. Nominally, the table is divided into 17 &quot;planes&quot; of
  ///     65,536 characters each, separating latin-based languages from asian language and
  ///     from funny poop emojis, but that part is only important for font designers.
  ///   </para>
  ///   <para>
  ///     An index into the unicode table is called a &quot;code point&quot;. So what used to
  ///     be a characters in an ASCII string are now code points in a unicode string.
  ///     The easiest way to store them would be to just keep an array of 32 bit integers,
  ///     each sufficient to hold one code point. That's precisely what UTF-32 is. While easy
  ///     to deal with, its downsides are wasted space and endian issues.
  ///   </para>
  ///   <para>
  ///     Enter UTF-8. It is a variable-length encoding where the first byte tells the number
  ///     of bytes that follow, up to 3. Amusingly, if the first byte's uppermost bit is unset,
  ///     this indicates a single-byte code point using 7 bits which happen to be mapped to
  ///     ASCII in a 1:1 fashion, in other words, any 7-bit ASCII string is a valid UTF-8
  ///     string. Consisting of only bytes, it isn't prone to endian issues.
  ///   </para>
  ///   <para>
  ///     Cool fact: in UTF-8 code points requiring 2, 3 or 4 bytes to encode, all of
  ///     the bytes have their highest bit set. That means that no single byte will intrude
  ///     into the 7-bit ASCII range. So if, for example, the byte 0x2f, '/', a path separator,
  ///     appears in the bytes of an UTF-8 string, it *is* the path separator since no
  ///     follow-up-byte in a 2, 3 or 4 byte code point can ever use the values 0x00-0x7f.
  ///     This allows UTF-8 to harmlessly pass through a lot of old software and/or code.
  ///   </para>
  ///   <para>
  ///     UTF-16 combines the worst of either: endian issues and wasted space. So naturally
  ///     Microsoft used it for all unicode in Windows. A code point is represented by one or
  ///     two 16 bit integers, again using the leading integer's high bits to indicate whether
  ///     the code point is complete or formed together with the 16 bit integer that follows.
  ///     Lots of Windows software, holds the opinion that one 16 bit integer, aka one wchar_t,
  ///     is one glyph, which tends to work until you localize to Asian languages.
  ///   </para>
  ///   <para>
  ///     One last confusing thing: whenever I write that UTF-8 encodes unicode code points
  ///     as 1-4 bytes, UTF-16 as one or two 16 bit integers and UTF-32 as a 32 bit integer,
  ///     the correct term in place of &quot;bytes&quot; and &quot;integers&quot; would be
  ///     &quot;characters&quot;. That's why in C++ the new data types are
  ///     <code>char8_t</code>, <code>char16_t</code> and <code>char32_t</code>.
  ///     So &quot;character&quot; has been (re-?)defined to mean &quot;encoding atom&quot;
  ///     and it is not always enough to represent an entire letter (aka code point).
  ///   </para>
  ///   <para>
  ///     A series of characters encoding a unicode code point is called a sequence.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE UnicodeHelper {

    /// <summary>UTF-8 character of which either 1, 2, 3 or 4 specify one codepoint</summary>
    /// <remarks>
    ///   Under C++20, this will be a native type like char16_t and char32_t. There will also
    ///   be an std::u8string using this character type to unambiguously indicate that
    ///   the contents of the string are supposed to be UTF-8 encoded.
    /// </remarks>
#if defined(__cpp_char8_t)
    public: typedef char8_t Char8Type;
#else
    public: typedef unsigned char Char8Type;
#endif

    /// <summary>The symbol used to indicate a code point is invalid or corrupted</summary>
    public: static const constexpr char32_t ReplacementCodePoint = char32_t(0xFFFD);

    /// <summary>Checks whether the specified unicode code point is valid</summary>
    /// <param name="codePoint">Code point that will be checked</param>
    /// <returns>True if the code point is valid, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsValidCodePoint(char32_t codePoint);

    /// <summary>
    ///   Returns the number of characters in a sequence by looking at the lead character
    /// </summary>
    /// <param name="leadCharacter">Lead character of the UTF-8 sequence</param>
    /// <returns>
    ///   The length of the sequence or <code>std::size_t(-1)</code> if the character
    ///   is not the lead character of a sequence (or is not valid UTF-8 at all).
    /// </returns>
    /// <remarks>
    ///   This method can be used to figure out if a character is the lead character, too.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t GetSequenceLength(
      Char8Type leadCharacter
    );

    /// <summary>
    ///   Returns the number of characters in a sequence by looking at the lead character
    /// </summary>
    /// <param name="leadCharacter">Lead character of the UTF-16 sequence</param>
    /// <returns>
    ///   The length of the sequence or <code>std::size_t(-1)</code> if the character
    ///   is not the lead character of a sequence (or is not valid UTF-16 at all).
    /// </returns>
    /// <remarks>
    ///   This method can be used to figure out if a character is the lead character, too.
    ///   It doesn't do any big/little endian conversion. If you know the input is from
    ///   in the endianness opposite of the current platform, byte-swap each char16_t.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t GetSequenceLength(
      char16_t leadCharacter
    );

    /// <summary>
    ///   Counts the number of UTF-8 characters needed to represent a code point
    /// </summary>
    /// <param name="codePoint">
    ///   Code point for which the needed UTF-8 characters will be counted
    /// </param>
    /// <returns>The number of characters needed to encode the code point in UTF-8</returns>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t CountUtf8Characters(
      char32_t codePoint
    );

    /// <summary>
    ///   Counts the number of UTF-16 characters needed to represent a code point
    /// </summary>
    /// <param name="codePoint">
    ///   Code point for which the needed UTF-16 characters will be counted
    /// </param>
    /// <returns>The number of characters needed to encode the code point in UTF-16</returns>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t CountUtf16Characters(
      char32_t codePoint
    );

    /// <summary>Reads a code point from a variable-length UTF-8 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-8 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-8 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-8 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      const Char8Type *&current, const Char8Type *end
    );

    /// <summary>Reads a code point from a variable-length UTF-8 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-8 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-8 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-8 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      Char8Type *&current, const Char8Type *end
    );

    /// <summary>Reads a code point from a variable-length UTF-16 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-16 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-16 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-16 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      const char16_t *&current, const char16_t *end
    );

    /// <summary>Reads a code point from a variable-length UTF-16 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-16 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-16 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-16 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      char16_t *&current, const char16_t *end
    );

    /// <summary>Reads a code point from a UTF-32 character</summary>
    /// <param name="current">
    ///   Address of the UTF-32 character, will be updated to the next character
    ///   if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-32 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-16 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      const char32_t *&current, const char32_t *end
    );

    /// <summary>Reads a code point from a UTF-32 character</summary>
    /// <param name="current">
    ///   Address of the UTF-32 character, will be updated to the next character
    ///   if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-32 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-16 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      char32_t *&current, const char32_t *end
    );

    /// <summary>Encodes the specified code point into UTF-8 characters</summary>
    /// <param name="codePoint">Code point that will be encoded as UTF-8</param>
    /// <param name="target">
    ///   Address at which the UTF-8 characters will be deposited. Needs to have at
    ///   least 4 bytes of usable space and will be moved to after the encoded characters
    /// </param>
    /// <returns>
    ///   The number of characters that have been encoded or std::size_t(-1) if
    ///   you specified an invalid code point.
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::size_t WriteCodePoint(
      Char8Type *&target, char32_t codePoint
    );

    /// <summary>Encodes the specified code point into UTF-16 characters</summary>
    /// <param name="codePoint">Code point that will be encoded as UTF-16</param>
    /// <param name="target">
    ///   Address at which the UTF-16 characters will be deposited. Needs to have at
    ///   least 4 bytes of usable space and will be moved to after the encoded characters
    /// </param>
    /// <returns>
    ///   The number of characters that have been encoded or std::size_t(-1) if
    ///   you specified an invalid code point.
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::size_t WriteCodePoint(
      char16_t *&target, char32_t codePoint
    );

    /// <summary>Encodes the specified code point into UTF-32 characters</summary>
    /// <param name="codePoint">Code point that will be encoded as UTF-32</param>
    /// <param name="target">
    ///   Address at which the UTF-16 characters will be deposited. Needs to have at
    ///   least 4 bytes of usable space and will be moved to after the encoded characters
    /// </param>
    /// <returns>
    ///   The number of characters that have been encoded or std::size_t(-1) if
    ///   you specified an invalid code point.
    /// </returns>
    public: NUCLEX_SUPPORT_API static inline std::size_t WriteCodePoint(
      char32_t *&target, char32_t codePoint
    );

    /// <summary>Converts the specified Unicode code point to folded lowercase</summary>
    /// <param name="codePoint">
    ///   Unicode code point that will be converted to folded lowercase
    /// </param>
    /// <returns>The character or its folded lowercase equivalent</returns>
    /// <remarks>
    ///   <para>
    ///     Folded lowercase is a special variant of lowercase that will result in a string of
    ///     equal or shorter length when encoded to UTF-8 or UTF-16. It is not intended for
    ///     display and some mappings may lead to incorrect lowercase characters for such.
    ///   </para>
    ///   <para>
    ///     Comparing the case-folded translations of two strings will produce the result of
    ///     a case-insensitive comparison. This makes case folding very useful for case
    ///     insensitive comparison logic and associative containers which can store
    ///     pre-case-folded strings for their indexes if they need to be case insensitive.
    ///   </para>
    ///   <para>
    ///     Warning: really, don't use this for displayed strings. It may even replace
    ///     lowercase characters with something weird in case their UTF-8-encoded code point
    ///     would be longer than its uppercase variant.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static char32_t ToFoldedLowercase(char32_t codePoint);

  };

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool UnicodeHelper::IsValidCodePoint(char32_t codePoint) {
    return (
      (codePoint < 0xD800) ||
      (
        (codePoint >= 0xE000) &&
        (codePoint < 1114111)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::GetSequenceLength(Char8Type leadCharacter) {
    if(leadCharacter < 128) {
      return 1;
    } else if((leadCharacter & 0xE0) == 0xC0) {
      return 2;
    } else if((leadCharacter & 0xF0) == 0xE0) {
      return 3;
    } else if((leadCharacter & 0xF8) == 0xF0) {
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::GetSequenceLength(char16_t leadCharacter) {
    if(leadCharacter < char16_t(0xD800)) {
      return 1; // Single character code point, below surrogate range
    } else if(leadCharacter >= char16_t(0xE000)) {
      return 1; // Single character code point, above surrogate range
    } else if(leadCharacter < char16_t(0xDC00)) {
      return 2; // Two-character code point, lead surrogate
    } else {
      return std::size_t(-1); // It's a trail surrogate, thus no lead character
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::CountUtf8Characters(char32_t codePoint) {
    if(codePoint < 128) {
      return 1;
    } else if(codePoint < 2048) {
      return 2;
    } else if(codePoint < 65536) {
      return 3;
    } else if(codePoint < 1114111) {
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::CountUtf16Characters(char32_t codePoint) {
    if(codePoint < 0xD800) {
      return 1;
    } else if((codePoint >= 0xE000) && (codePoint < 1114111)) {
      return 2;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline char32_t UnicodeHelper::ReadCodePoint(const char32_t *&current, const char32_t *end) {
    assert((current < end) && u8"At least one UTF-32 character of input must be available");
    NUCLEX_SUPPORT_NDEBUG_UNUSED(end);

    char32_t codePoint = *current;
    ++current;
    return codePoint;
  }

  // ------------------------------------------------------------------------------------------- //

  inline char32_t UnicodeHelper::ReadCodePoint(char32_t *&current, const char32_t *end) {
    assert((current < end) && u8"At least one UTF-32 character of input must be available");
    NUCLEX_SUPPORT_NDEBUG_UNUSED(end);

    char32_t codePoint = *current;
    ++current;
    return codePoint;
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::size_t UnicodeHelper::WriteCodePoint(Char8Type *&target, char32_t codePoint) {
    if(codePoint < 128) {
      *target = static_cast<Char8Type>(codePoint);
      ++target;
      return 1;
    } else if(codePoint < 2048) {
      *target = Char8Type(0xC0) | static_cast<Char8Type>(codePoint >> 6);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>(codePoint & 0x3F);
      ++target;
      return 2;
    } else if(codePoint < 65536) {
      *target = Char8Type(0xE0) | static_cast<Char8Type>(codePoint >> 12);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>((codePoint >> 6) & 0x3F);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>(codePoint & 0x3F);
      ++target;
      return 3;
    } else if(codePoint < 1114111) {
      *target = Char8Type(0xF0) | static_cast<Char8Type>(codePoint >> 18);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>((codePoint >> 12) & 0x3F);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>((codePoint >> 6) & 0x3F);
      ++target;
      *target = Char8Type(0x80) | static_cast<Char8Type>(codePoint & 0x3F);
      ++target;
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::size_t UnicodeHelper::WriteCodePoint(char16_t *&target, char32_t codePoint) {
    if(codePoint < 65536) {
      assert(
        ((codePoint < 0xDC00) || (codePoint >= 0xE000)) &&
        u8"Unicode code point has to be outside surrogate range (0xDC00-0xDFFF)"
      );
      *target = static_cast<char16_t>(codePoint);
      ++target;
      return 1;
    } else if(codePoint < 1114111) {
      codePoint -= char32_t(65536);
      *(target) = 0xD800 | static_cast<char16_t>(codePoint >> 10);
      *(target + 1) = 0xDC00 | static_cast<char16_t>(codePoint & 0x03FF);
      target += 2;
      return 2;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::size_t UnicodeHelper::WriteCodePoint(char32_t *&target, char32_t codePoint) {
    *target = codePoint;
    ++target;
    return 1;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H
