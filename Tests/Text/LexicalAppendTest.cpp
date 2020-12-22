#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <cmath>
#include <clocale>

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendBooleanToString) {
    std::string trueString(u8"is ");
    lexical_append(trueString, true);
    EXPECT_EQ(trueString.length(), 7U);
    EXPECT_EQ(trueString, u8"is true");

    std::string falseString(u8"might be ");
    lexical_append(falseString, false);
    EXPECT_EQ(falseString.length(), 14U);
    EXPECT_EQ(falseString, u8"might be false");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendBooleanToCharArray) {
    char characters[8] = { 0 };

    characters[0] = 122;
    characters[5] = 123;
    std::size_t characterCount = lexical_append(characters + 1, 4U, true);
    EXPECT_EQ(characterCount, 4U);

    EXPECT_EQ(characters[0], 122);
    EXPECT_EQ(characters[1], 't');
    EXPECT_EQ(characters[2], 'r');
    EXPECT_EQ(characters[3], 'u');
    EXPECT_EQ(characters[4], 'e');
    EXPECT_EQ(characters[5], 123);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForBoolean) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, true);
    EXPECT_EQ(characterCount, 4U);

    characterCount = lexical_append(characters, 1U, false);
    EXPECT_EQ(characterCount, 5U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendCharactersToString) {
    const char *appended = u8"Hello World";

    std::string messageString("Hello Sky, ");
    lexical_append(messageString, appended);
    EXPECT_EQ(messageString.length(), 22U);
    EXPECT_EQ(messageString, u8"Hello Sky, Hello World");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendCharactersToCharArray) {
    const char *appended = u8"Hello World";
    char characters[14] = { 0 };

    characters[0] = 123;
    characters[12] = 124;
    std::size_t characterCount = lexical_append(characters + 1, 11U, appended);
    EXPECT_EQ(characterCount, 11U);

    EXPECT_EQ(characters[0], 123);
    EXPECT_EQ(characters[1], 'H');
    EXPECT_EQ(characters[2], 'e');
    EXPECT_EQ(characters[3], 'l');
    EXPECT_EQ(characters[4], 'l');
    EXPECT_EQ(characters[5], 'o');
    EXPECT_EQ(characters[6], ' ');
    EXPECT_EQ(characters[7], 'W');
    EXPECT_EQ(characters[8], 'o');
    EXPECT_EQ(characters[9], 'r');
    EXPECT_EQ(characters[10], 'l');
    EXPECT_EQ(characters[11], 'd');
    EXPECT_EQ(characters[12], 124);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForCharacters) {
    const char *appended = u8"Hello World";
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, appended);
    EXPECT_EQ(characterCount, 11U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendNullPointerToString) {
    const char *appended = nullptr;

    std::string resultString(u8"The appended part is a ");
    lexical_append(resultString, appended);

    EXPECT_EQ(resultString.length(), 32U);
    EXPECT_EQ(resultString, u8"The appended part is a <nullptr>");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendNullPointerToCharArray) {
    const char *appended = nullptr;
    char characters[14] = { 0 };

    characters[0] = 124;
    characters[10] = 125;
    std::size_t characterCount = lexical_append(characters + 1, 9U, appended);
    EXPECT_EQ(characterCount, 9U);

    EXPECT_EQ(characters[0], 124);
    EXPECT_EQ(characters[1], '<');
    EXPECT_EQ(characters[2], 'n');
    EXPECT_EQ(characters[3], 'u');
    EXPECT_EQ(characters[4], 'l');
    EXPECT_EQ(characters[5], 'l');
    EXPECT_EQ(characters[6], 'p');
    EXPECT_EQ(characters[7], 't');
    EXPECT_EQ(characters[8], 'r');
    EXPECT_EQ(characters[9], '>');
    EXPECT_EQ(characters[10], 125);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForNullPointer) {
    const char *appended = nullptr;
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, appended);
    EXPECT_EQ(characterCount, 9U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt8ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::uint8_t(234));

    EXPECT_EQ(resultString.length(), 16U);
    EXPECT_EQ(resultString, u8"Value equals 234");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt8ToCharacterArray) {
    char characters[5] = { 0 };

    characters[0] = 125;
    characters[4] = 126;
    std::size_t characterCount = lexical_append(characters + 1, 3U, std::uint8_t(234));
    EXPECT_EQ(characterCount, 3U);

    EXPECT_EQ(characters[0], 125);
    EXPECT_EQ(characters[1], '2');
    EXPECT_EQ(characters[2], '3');
    EXPECT_EQ(characters[3], '4');
    EXPECT_EQ(characters[4], 126);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt8) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::uint8_t(9));
    EXPECT_EQ(characterCount, 1U);
    characterCount = lexical_append(characters, 1U, std::uint8_t(99));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::uint8_t(100));
    EXPECT_EQ(characterCount, 3U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt8ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::int8_t(-123));

    EXPECT_EQ(resultString.length(), 17U);
    EXPECT_EQ(resultString, u8"Value equals -123");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt8ToCharacterArray) {
    char characters[6] = { 0 };

    characters[0] = 126;
    characters[5] = 127;
    std::size_t characterCount = lexical_append(characters + 1, 4U, std::int8_t(-123));
    EXPECT_EQ(characterCount, 4U);

    EXPECT_EQ(characters[0], 126);
    EXPECT_EQ(characters[1], '-');
    EXPECT_EQ(characters[2], '1');
    EXPECT_EQ(characters[3], '2');
    EXPECT_EQ(characters[4], '3');
    EXPECT_EQ(characters[5], 127);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt8) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::int8_t(-9));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::int8_t(-99));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::int8_t(-100));
    EXPECT_EQ(characterCount, 4U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt16ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::uint16_t(56789));

    EXPECT_EQ(resultString.length(), 18U);
    EXPECT_EQ(resultString, u8"Value equals 56789");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt16ToCharacterArray) {
    char characters[7] = { 0 };

    characters[0] = 120;
    characters[6] = 121;
    std::size_t characterCount = lexical_append(characters + 1, 5U, std::uint16_t(56789));
    EXPECT_EQ(characterCount, 5U);

    EXPECT_EQ(characters[0], 120);
    EXPECT_EQ(characters[1], '5');
    EXPECT_EQ(characters[2], '6');
    EXPECT_EQ(characters[3], '7');
    EXPECT_EQ(characters[4], '8');
    EXPECT_EQ(characters[5], '9');
    EXPECT_EQ(characters[6], 121);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt16) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::uint16_t(9));
    EXPECT_EQ(characterCount, 1U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(10));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(99));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(100));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(999));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(1000));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(9999));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::uint16_t(10000));
    EXPECT_EQ(characterCount, 5U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt16ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::int16_t(-23456));

    EXPECT_EQ(resultString.length(), 19U);
    EXPECT_EQ(resultString, u8"Value equals -23456");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt16ToCharacterArray) {
    char characters[8] = { 0 };

    characters[0] = 121;
    characters[7] = 122;
    std::size_t characterCount = lexical_append(characters + 1, 6U, std::int16_t(-23456));
    EXPECT_EQ(characterCount, 6U);

    EXPECT_EQ(characters[0], 121);
    EXPECT_EQ(characters[1], '-');
    EXPECT_EQ(characters[2], '2');
    EXPECT_EQ(characters[3], '3');
    EXPECT_EQ(characters[4], '4');
    EXPECT_EQ(characters[5], '5');
    EXPECT_EQ(characters[6], '6');
    EXPECT_EQ(characters[7], 122);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt16) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::int16_t(-9));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-99));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-100));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-999));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-1000));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-9999));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int16_t(-10000));
    EXPECT_EQ(characterCount, 6U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt32ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::uint32_t(1234567890));

    EXPECT_EQ(resultString.length(), 23U);
    EXPECT_EQ(resultString, u8"Value equals 1234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt32ToCharacterArray) {
    char characters[12] = { 0 };

    {
      characters[0] = 122;
      characters[2] = 123;
      std::size_t characterCount = lexical_append(characters + 1, 1U, std::uint32_t(0));
      EXPECT_EQ(characterCount, 1U);

      EXPECT_EQ(characters[0], 122);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 123);
    }

    {
      characters[0] = 123;
      characters[11] = 124;
      std::size_t characterCount = lexical_append(characters + 1, 10U, std::uint32_t(1234567890));
      EXPECT_EQ(characterCount, 10U);

      EXPECT_EQ(characters[0], 123);
      EXPECT_EQ(characters[1], '1');
      EXPECT_EQ(characters[2], '2');
      EXPECT_EQ(characters[3], '3');
      EXPECT_EQ(characters[4], '4');
      EXPECT_EQ(characters[5], '5');
      EXPECT_EQ(characters[6], '6');
      EXPECT_EQ(characters[7], '7');
      EXPECT_EQ(characters[8], '8');
      EXPECT_EQ(characters[9], '9');
      EXPECT_EQ(characters[10], '0');
      EXPECT_EQ(characters[11], 124);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt32) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::uint32_t(0));
    EXPECT_EQ(characterCount, 1U);

    characterCount = lexical_append(characters, 1U, std::uint32_t(9));
    EXPECT_EQ(characterCount, 1U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(99));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(100));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(999));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(1000));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(9999));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(99999));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(10000));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(100000));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(999999));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(1000000));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(9999999));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(10000000));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(99999999));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(100000000));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(999999999));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::uint32_t(1000000000));
    EXPECT_EQ(characterCount, 10U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt32ToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, std::int32_t(0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, std::int32_t(1234567890));
    EXPECT_EQ(resultString.length(), 24U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890");

    lexical_append(resultString, std::int32_t(-1234567890));
    EXPECT_EQ(resultString.length(), 35U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890-1234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt32ToCharacterArray) {
    char characters[13] = { 0 };

    {
      characters[0] = 124;
      characters[2] = 125;
      std::size_t characterCount = lexical_append(characters + 1, 1U, std::int32_t(0));
      EXPECT_EQ(characterCount, 1U);
      EXPECT_EQ(characters[0], 124);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 125);
    }

    {
      characters[0] = 125;
      characters[11] = 126;
      std::size_t characterCount = lexical_append(characters + 1, 10U, std::int32_t(1234567890));
      EXPECT_EQ(characterCount, 10U);

      EXPECT_EQ(characters[0], 125);
      EXPECT_EQ(characters[1], '1');
      EXPECT_EQ(characters[2], '2');
      EXPECT_EQ(characters[3], '3');
      EXPECT_EQ(characters[4], '4');
      EXPECT_EQ(characters[5], '5');
      EXPECT_EQ(characters[6], '6');
      EXPECT_EQ(characters[7], '7');
      EXPECT_EQ(characters[8], '8');
      EXPECT_EQ(characters[9], '9');
      EXPECT_EQ(characters[10], '0');
      EXPECT_EQ(characters[11], 126);
    }

    {
      characters[0] = 126;
      characters[12] = 127;
      std::size_t characterCount = lexical_append(characters + 1, 11U, std::int32_t(-1234567890));
      EXPECT_EQ(characterCount, 11U);

      EXPECT_EQ(characters[0], 126);
      EXPECT_EQ(characters[1], '-');
      EXPECT_EQ(characters[2], '1');
      EXPECT_EQ(characters[3], '2');
      EXPECT_EQ(characters[4], '3');
      EXPECT_EQ(characters[5], '4');
      EXPECT_EQ(characters[6], '5');
      EXPECT_EQ(characters[7], '6');
      EXPECT_EQ(characters[8], '7');
      EXPECT_EQ(characters[9], '8');
      EXPECT_EQ(characters[10], '9');
      EXPECT_EQ(characters[11], '0');
      EXPECT_EQ(characters[12], 127);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt32) {
    char characters[1] = { 0 };

    std::size_t characterCount = lexical_append(characters, 1U, std::int32_t(0));
    EXPECT_EQ(characterCount, 1U);

    characterCount = lexical_append(characters, 1U, std::int32_t(-9));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-99));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-100));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-999));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-1000));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-9999));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-10000));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-99999));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-100000));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-999999));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-1000000));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-9999999));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-10000000));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-99999999));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-100000000));
    EXPECT_EQ(characterCount, 10U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-999999999));
    EXPECT_EQ(characterCount, 10U);
    characterCount = lexical_append(characters, 1U, std::int32_t(-1000000000));
    EXPECT_EQ(characterCount, 11U);

    characterCount = lexical_append(characters, 1U, std::int32_t(9));
    EXPECT_EQ(characterCount, 1U);
    characterCount = lexical_append(characters, 1U, std::int32_t(99));
    EXPECT_EQ(characterCount, 2U);
    characterCount = lexical_append(characters, 1U, std::int32_t(100));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::int32_t(999));
    EXPECT_EQ(characterCount, 3U);
    characterCount = lexical_append(characters, 1U, std::int32_t(1000));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int32_t(9999));
    EXPECT_EQ(characterCount, 4U);
    characterCount = lexical_append(characters, 1U, std::int32_t(99999));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int32_t(10000));
    EXPECT_EQ(characterCount, 5U);
    characterCount = lexical_append(characters, 1U, std::int32_t(100000));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::int32_t(999999));
    EXPECT_EQ(characterCount, 6U);
    characterCount = lexical_append(characters, 1U, std::int32_t(1000000));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::int32_t(9999999));
    EXPECT_EQ(characterCount, 7U);
    characterCount = lexical_append(characters, 1U, std::int32_t(10000000));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::int32_t(99999999));
    EXPECT_EQ(characterCount, 8U);
    characterCount = lexical_append(characters, 1U, std::int32_t(100000000));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::int32_t(999999999));
    EXPECT_EQ(characterCount, 9U);
    characterCount = lexical_append(characters, 1U, std::int32_t(1000000000));
    EXPECT_EQ(characterCount, 10U);

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
