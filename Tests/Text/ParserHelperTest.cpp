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

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanDetectSingleSingleByteWhitespace) {
    EXPECT_TRUE(ParserHelper::IsWhitespace(char(' ')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char('\t')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char('\r')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char('\n')));

    EXPECT_FALSE(ParserHelper::IsWhitespace(char('a')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char('?')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char('\'')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char(0)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanDetectUtf32Whitespace) {
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(U' ')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(U'\t')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(0x00a0)));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(0x2003)));

    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(U'a')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(U'Ø')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(0x200b)));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(0)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanSkipWhitespaces) {
    std::string text(u8"\t Hellø Ünicøde Wórld ", 26);
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start + text.length();

    // Beginning w/multiple whitespaces
    {
      const std::uint8_t *current = start;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current - start, 2U);
    }

    // On letter
    {
      const std::uint8_t *current = start + 3;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current - start, 3U);
    }

    // Before two-byte encoded code point
    {
      const std::uint8_t *current = start + 20;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current - start, 20U);
    }

    // On last character
    {
      const std::uint8_t *current = start + 25;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current - start, 26U);
    }

    // Past last character
    {
      const std::uint8_t *current = start + 26;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current - start, 26U);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanSkipNonWhitespaces) {
    std::string text(u8"\t Hellø Ünicøde Wórld ", 26);
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start + text.length();

    // First whitespace at beginning
    {
      const std::uint8_t *current = start;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 0U);
    }

    // Second whitespace at beginning
    {
      const std::uint8_t *current = start + 1;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 1U);
    }

    // First whitespace between words
    {
      const std::uint8_t *current = start + 2;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 8U);
    }

    // Second whitespace between words
    {
      const std::uint8_t *current = start + 9;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 18U);
    }

    // Past last character
    {
      const std::uint8_t *current = start + 19;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 25U);
    }

    // On string end
    {
      const std::uint8_t *current = start + 26;
      ParserHelper::SkipNonWhitespace(current, end);
      EXPECT_EQ(current - start, 26U);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanDetectBlankAndEmptyStrings) {
    EXPECT_TRUE(ParserHelper::IsBlankOrEmpty(std::string()));
    EXPECT_TRUE(ParserHelper::IsBlankOrEmpty(std::string(u8" ")));
    EXPECT_TRUE(ParserHelper::IsBlankOrEmpty(std::string(u8"\t")));
    EXPECT_TRUE(ParserHelper::IsBlankOrEmpty(std::string(u8" \t\t ")));

    EXPECT_FALSE(ParserHelper::IsBlankOrEmpty(std::string(u8" ? ")));
    EXPECT_FALSE(ParserHelper::IsBlankOrEmpty(std::string(u8"\t a")));
    EXPECT_FALSE(ParserHelper::IsBlankOrEmpty(std::string(u8"a \t")));
    EXPECT_FALSE(ParserHelper::IsBlankOrEmpty(std::string(u8"Hello")));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanFindWordInString) {
    std::string text(u8"\t Hellø \r\n Ünicøde Wórld", 28);
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start + text.length();

    // First whitespace at beginning
    {
      std::string_view word;

      const std::uint8_t *current = start;
      ParserHelper::FindWord(current, end, &word);
      EXPECT_EQ(current - start, 2U);
      EXPECT_TRUE(word == u8"Hellø");
    }

    // In the middle of a word
    {
      std::string_view word;

      const std::uint8_t *current = start + 14;
      ParserHelper::FindWord(current, end, &word);
      EXPECT_EQ(current - start, 14U);
      EXPECT_TRUE(word == u8"nicøde");
    }

    // Word in which the string ends
    {
      std::string_view word;

      const std::uint8_t *current = start + 21;
      ParserHelper::FindWord(current, end, &word);
      EXPECT_EQ(current - start, 22U);
      EXPECT_TRUE(word == u8"Wórld");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanFindLineInString) {
    std::string text(
      u8"Unix line break\nWindows line break\r\nMac line break\rNo line break", 64
    );
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start + text.length();

    // First Unix-style line break
    {
      std::string_view line;

      const std::uint8_t *current = start;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 16U);
      EXPECT_TRUE(line == u8"Unix line break");
    }

    // Second Windows-style line break
    {
      std::string_view line;

      const std::uint8_t *current = start + 16;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 36U);
      EXPECT_TRUE(line == u8"Windows line break");
    }

    // Third MacOS-style line break
    {
      std::string_view line;

      const std::uint8_t *current = start + 36;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 51U);
      EXPECT_TRUE(line == u8"Mac line break");
    }

    // Fourth line running against the end of the string
    {
      std::string_view line;

      const std::uint8_t *current = start + 51;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 64U);
      EXPECT_TRUE(line == u8"No line break");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, FindLineHandlesEmptyStrings) {
    std::string text(u8"", 0);
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start;

    std::string_view line;
    ParserHelper::FindLine(start, end, &line);

    EXPECT_EQ(start, end);
    EXPECT_TRUE(line.empty());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, FindLineHandlesEmptyLines) {
    std::string text(u8"Linux\n\nWindows\r\n\r\nMac\r\r", 23);
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text.c_str());
    const std::uint8_t *end = start + text.length();

    // Empty line using Linux style line ending
    {
      std::string_view line;

      const std::uint8_t *current = start;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 6U);
      EXPECT_TRUE(line == u8"Linux");
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 7U);
      EXPECT_TRUE(line.empty());
    }

    // Empty line using Windows style line ending
    {
      std::string_view line;

      const std::uint8_t *current = start + 7;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 16U);
      EXPECT_TRUE(line == u8"Windows");
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 18U);
      EXPECT_TRUE(line.empty());
    }

    // Empty line using old Mac-style line ending
    {
      std::string_view line;

      const std::uint8_t *current = start + 18;
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 22U);
      EXPECT_TRUE(line == u8"Mac");
      ParserHelper::FindLine(current, end, &line);
      EXPECT_EQ(current - start, 23U);
      EXPECT_TRUE(line.empty());
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
