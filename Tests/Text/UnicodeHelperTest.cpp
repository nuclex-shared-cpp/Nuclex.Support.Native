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

#include "Nuclex/Support/Text/UnicodeHelper.h"

#include <cstdint> // for std::uint8_t

#include <gtest/gtest.h>

namespace Nuclex::Support::Text {

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

  TEST(UnicodeHelperTest, CalculatesUtf16CharacterCount) {
    char32_t asciiCodePoint = U'x';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(asciiCodePoint), 1U);

    char32_t centCodePoint = U'¢';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(centCodePoint), 1U);

    char32_t euroCodePoint = U'€';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(euroCodePoint), 1U);

    char32_t gothicCodePoint = U'𐍈';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(gothicCodePoint), 2U);

    char32_t surrogateCodePoint = char32_t(0xDD00); // surrogate range
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(surrogateCodePoint), std::size_t(-1));

    char32_t invalidCodePoint = 1114111;
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(invalidCodePoint), std::size_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, TellsSequenceLengthFromUtf8LeadCharacter) {
    std::u8string ascii(u8"A");
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(ascii[0]), 1U
    );

    std::u8string cents(u8"¢");
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(cents[0]), 2U
    );

    std::u8string euros(u8"€");
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(euros[0]), 3U
    );

    std::u8string gothic(u8"𐍈");
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(gothic[0]), 4U
    );

    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(char8_t(0x80)), std::size_t(-1)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf8) {
    {
      std::u8string ascii(u8"A");
      const char8_t *start = ascii.c_str();
      const char8_t *end = start + ascii.length();
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'A');
      EXPECT_EQ(start, end);
    }

    {
      std::u8string cents(u8"¢");
      const char8_t *start = cents.c_str();
      const char8_t *end = start + cents.length();
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'¢');
      EXPECT_EQ(start, end);
    }

    {
      std::u8string euros(u8"€");
      const char8_t *start = euros.c_str();
      const char8_t *end = start + euros.length();
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'€');
      EXPECT_EQ(start, end);
    }

    {
      std::u8string gothic(u8"𐍈");
      const char8_t *start = gothic.c_str();
      const char8_t *end = start + gothic.length();
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'𐍈');
      EXPECT_EQ(start, end);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadingInvalidCodePointFromUtf8Fails) {

    // Invalid second byte should be detected
    {
      char8_t invalid[] = u8"𐍈";
      invalid[1] = char8_t(0xC0); // 0b11xxxxxx
      const char8_t *start = invalid;
      const char8_t *end = start + sizeof(invalid);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, static_cast<const char8_t *>(invalid));
    }

    // Invalid length (5 bytes, possible by encoding, but always invalid since
    // it's either an out-of-range code point or an overlong code point).
    {
      char8_t invalid[] = u8"𐍈";
      invalid[0] = char8_t(0xF8); // 0b11111000
      const char8_t *start = invalid;
      const char8_t *end = start + sizeof(invalid);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, static_cast<const char8_t *>(invalid));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf16) {
    {
      const char16_t ascii[] = u"A";
      const char16_t *start = reinterpret_cast<const char16_t *>(ascii);
      const char16_t *end = reinterpret_cast<const char16_t *>(ascii) + sizeof(ascii);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'A');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(ascii) + 1);
    }

    {
      const char16_t cent[] = u"¢";
      const char16_t *start = reinterpret_cast<const char16_t *>(cent);
      const char16_t *end = reinterpret_cast<const char16_t *>(cent) + sizeof(cent);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'¢');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(cent) + 1);
    }

    {
      const char16_t euro[] = u"€";
      const char16_t *start = reinterpret_cast<const char16_t *>(euro);
      const char16_t *end = reinterpret_cast<const char16_t *>(euro) + sizeof(euro);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'€');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(euro) + 1);
    }

    {
      const char16_t gothic[] = u"𐍈";
      const char16_t *start = reinterpret_cast<const char16_t *>(gothic);
      const char16_t *end = reinterpret_cast<const char16_t *>(gothic) + sizeof(gothic);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'𐍈');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(gothic) + 2);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadingInvalidCodePointFromUtf16Fails) {
    {
      char16_t gothic[] = u"𐍈";
      { // Flip lead/trail surrogates
        char16_t temp = gothic[0];
        gothic[0] = gothic[1];
        gothic[1] = temp;
      }
      const char16_t *start = reinterpret_cast<const char16_t *>(gothic);
      const char16_t *end = reinterpret_cast<const char16_t *>(gothic) + sizeof(gothic);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(gothic));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, EncodesCodePointsToUtf8) {

    {
      char8_t ascii[4] = { 255, 255, 255, 255 };
      char8_t *start = ascii;
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'A');
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char8_t *>(ascii) + 1);
      EXPECT_EQ(ascii[0], u8'A');
      EXPECT_EQ(ascii[1], char8_t(255));
      EXPECT_EQ(ascii[2], char8_t(255));
      EXPECT_EQ(ascii[3], char8_t(255));
    }

    {
      char8_t cent[4] = { 255, 255, 255, 255 };
      char8_t *start = cent;
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'¢');
      EXPECT_EQ(count, 2U);
      EXPECT_EQ(start, reinterpret_cast<char8_t *>(cent) + 2);

      const char8_t expected[] = u8"¢";
      EXPECT_EQ(cent[0], expected[0]);
      EXPECT_EQ(cent[1], expected[1]);
      EXPECT_EQ(cent[2], char8_t(255));
      EXPECT_EQ(cent[3], char8_t(255));
    }

    {
      char8_t euro[4] = { 255, 255, 255, 255 };
      char8_t *start = euro;
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'€');
      EXPECT_EQ(count, 3U);
      EXPECT_EQ(start, reinterpret_cast<char8_t *>(euro) + 3);

      const char8_t expected[] = u8"€";
      EXPECT_EQ(euro[0], expected[0]);
      EXPECT_EQ(euro[1], expected[1]);
      EXPECT_EQ(euro[2], expected[2]);
      EXPECT_EQ(euro[3], char8_t(255));
    }

    {
      char8_t gothic[4] = { 255, 255, 255, 255 };
      char8_t *start = gothic;
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'𐍈');
      EXPECT_EQ(count, 4U);
      EXPECT_EQ(start, reinterpret_cast<char8_t *>(gothic) + 4);

      const char8_t expected[] = u8"𐍈";
      EXPECT_EQ(gothic[0], expected[0]);
      EXPECT_EQ(gothic[1], expected[1]);
      EXPECT_EQ(gothic[2], expected[2]);
      EXPECT_EQ(gothic[3], expected[3]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, EncodesCodePointsToUtf16) {
    {
      char16_t ascii[2] = { 65535, 65535};
      char16_t *start = reinterpret_cast<char16_t *>(ascii);
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'A');
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(ascii) + 1);
      EXPECT_EQ(ascii[0], u'A');
    }

    {
      char16_t cent[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(cent);
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'¢');
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(cent) + 1);

      const char16_t expected[] = u"¢";
      EXPECT_EQ(cent[0], expected[0]);
    }

    {
      char16_t euro[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(euro);
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'€');
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(euro) + 1);

      const char16_t expected[] = u"€";
      EXPECT_EQ(euro[0], expected[0]);
    }

    {
      char16_t gothic[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(gothic);
      std::size_t count = UnicodeHelper::WriteCodePoint(start, U'𐍈');
      EXPECT_EQ(count, 2U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(gothic) + 2);

      const char16_t expected[] = u"𐍈";
      EXPECT_EQ(gothic[0], expected[0]);
      EXPECT_EQ(gothic[1], expected[1]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, CaseFoldingAllowsCaseInsensitiveComparison) {
    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'A'),
      UnicodeHelper::ToFoldedLowercase(U'a')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'Ā'),
      UnicodeHelper::ToFoldedLowercase(U'ā')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'Ω'),
      UnicodeHelper::ToFoldedLowercase(U'ω')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'𑢰'),
      UnicodeHelper::ToFoldedLowercase(U'𑣐')
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
