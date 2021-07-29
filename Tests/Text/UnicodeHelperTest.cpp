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

#include "Nuclex/Support/Text/UnicodeHelper.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, DetectsInvalidCodePoints) {
    char32_t validCodePoint = U'Ø';
    EXPECT_TRUE(UnicodeHelper::IsValidCodePoint(validCodePoint));

    char32_t invalidCodePoint = 1114111; // the first invalid code point
    EXPECT_FALSE(UnicodeHelper::IsValidCodePoint(invalidCodePoint));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, CalculatesUtf8CharacterCount) {
    char32_t asciiCodePoint = U'x';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(asciiCodePoint), 1U);

    char32_t centCodePoint = U'¢';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(centCodePoint), 2U);

    char32_t euroCodePoint = U'€';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(euroCodePoint), 3U);

    char32_t gothicCodePoint = U'𐍈';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(gothicCodePoint), 4U);

    char32_t invalidCodePoint = 1114111;
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(invalidCodePoint), std::size_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, TellsSequenceLengthFromUtf8LeadCharacter) {
    const char *ascii = u8"A";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(
        *reinterpret_cast<const UnicodeHelper::char8_t *>(ascii)
      ),
      1U
    );

    const char *cents = u8"¢";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(
        *reinterpret_cast<const UnicodeHelper::char8_t *>(cents)
      ),
      2U
    );

    const char *euros = u8"€";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(
        *reinterpret_cast<const UnicodeHelper::char8_t *>(euros)
      ),
      3U
    );

    const char *gothic = u8"𐍈";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(
        *reinterpret_cast<const UnicodeHelper::char8_t *>(gothic)
      ),
      4U
    );

    EXPECT_EQ(UnicodeHelper::GetSequenceLength(UnicodeHelper::char8_t(0x80)), std::size_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf8Unchecked) {
    const char *ascii = u8"A";
    char32_t codePoint = UnicodeHelper::ReadCodePoint(
      reinterpret_cast<const UnicodeHelper::char8_t *>(ascii), 1
    );
    EXPECT_EQ(codePoint, U'A');

    const char *cents = u8"¢";
    codePoint = UnicodeHelper::ReadCodePoint(
      reinterpret_cast<const UnicodeHelper::char8_t *>(cents), 2
    );
    EXPECT_EQ(codePoint, U'¢');

    const char *euros = u8"€";
    codePoint = UnicodeHelper::ReadCodePoint(
      reinterpret_cast<const UnicodeHelper::char8_t *>(euros), 3
    );
    EXPECT_EQ(codePoint, U'€');

    const char *gothic = u8"𐍈";
    codePoint = UnicodeHelper::ReadCodePoint(
      reinterpret_cast<const UnicodeHelper::char8_t *>(gothic), 4
    );
    EXPECT_EQ(codePoint, U'𐍈');

    char invalid[] = u8"𐍈";
    *reinterpret_cast<std::uint8_t *>(&invalid[1]) = 0x80;
    codePoint = UnicodeHelper::ReadCodePoint(
      reinterpret_cast<const UnicodeHelper::char8_t *>(invalid), 4
    );
    EXPECT_NE(codePoint, char32_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf8Checked) {
    const char *ascii = u8"A";
    char32_t codePoint = UnicodeHelper::ReadCodePointChecked(
      reinterpret_cast<const UnicodeHelper::char8_t *>(ascii), 1
    );
    EXPECT_EQ(codePoint, U'A');

    const char *cents = u8"¢";
    codePoint = UnicodeHelper::ReadCodePointChecked(
      reinterpret_cast<const UnicodeHelper::char8_t *>(cents), 2
    );
    EXPECT_EQ(codePoint, U'¢');

    const char *euros = u8"€";
    codePoint = UnicodeHelper::ReadCodePointChecked(
      reinterpret_cast<const UnicodeHelper::char8_t *>(euros), 3
    );
    EXPECT_EQ(codePoint, U'€');

    const char *gothic = u8"𐍈";
    codePoint = UnicodeHelper::ReadCodePointChecked(
      reinterpret_cast<const UnicodeHelper::char8_t *>(gothic), 4
    );
    EXPECT_EQ(codePoint, U'𐍈');

    char invalid[] = u8"𐍈";
    *reinterpret_cast<std::uint8_t *>(&invalid[1]) = 0xC0; // 0b11xxxxxx
    codePoint = UnicodeHelper::ReadCodePointChecked(
      reinterpret_cast<const UnicodeHelper::char8_t *>(invalid), 4
    );
    EXPECT_EQ(codePoint, char32_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
