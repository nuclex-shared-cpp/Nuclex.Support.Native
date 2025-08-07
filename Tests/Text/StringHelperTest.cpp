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

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, RemovingNothingIsFine) {
    std::u8string expected(u8"This is a test", 14);

    // The main point of this test is to verify that there are no out-of-bounds
    // accesses (as would happen when reading from the empty substring).
    std::u8string test = expected;
    StringHelper::EraseSubstrings(test, std::u8string());

    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleOccurrenceOfSubstringCanBeRemoved) {
    std::u8string test(u8"This test did not succeed", 25);
    std::u8string expected(u8"This test did succeed", 21);

    StringHelper::EraseSubstrings(test, u8" not");

    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, WholeStringCanMatchSubstring) {
    std::u8string test(u8"Test", 4);
    std::u8string expected(u8"", 0);

    StringHelper::EraseSubstrings(test, u8"Test");

    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, DuplicateWhitespaceCanBeCollapsedWithoutTrim) {
    std::u8string test(u8" This  is   an example  ", 24);
    std::u8string expected(u8" This is an example ", 20);

    StringHelper::CollapseDuplicateWhitespace(test, false);

    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, DuplicateWhitespaceCanBeCollapsedWithTrim) {
    std::u8string test(u8"  This  is   an example ", 24);
    std::u8string expected(u8"This is an example", 18);

    StringHelper::CollapseDuplicateWhitespace(test, true);

    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, EmptyStringCanBeWhitespaceCollapsed) {
    std::u8string test(u8"", 0);
    std::u8string test2(test);
    std::u8string expected(u8"", 0);

    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_EQ(test, expected);

    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_EQ(test2, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleSpaceCanBeWhitespaceCollapsed) {
    std::u8string test(u8" ", 1);
    std::u8string test2(test);

    std::u8string expected(u8" ", 1);
    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_EQ(test, expected);

    std::u8string expected2(u8"", 0);
    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_EQ(test2, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SpacesOnlyCanBeWhitespaceCollapsed) {
    std::u8string test(u8"   ", 3);
    std::u8string test2(test);

    std::u8string expected(u8" ", 1);
    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_EQ(test, expected);

    std::u8string expected2(u8"", 0);
    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_EQ(test2, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, StringEndingInWhitespaceCanBeCollapsedWithTrim) {
    std::u8string test(u8"Hello World ", 12);
    std::u8string expected(u8"Hello World", 11);

    StringHelper::CollapseDuplicateWhitespace(test, true);
    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleCharacterSurvivesWhitespaceCollapse) {
    std::u8string test(u8"d", 1);
    std::u8string expected(u8"d", 1);

    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_EQ(test, expected);

    StringHelper::CollapseDuplicateWhitespace(test, true);
    ASSERT_EQ(test, expected);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, TrimmedEmptyStringIsStillEmpty) {
    std::u8string_view trimmedEmpty = StringHelper::GetTrimmed(std::u8string());
    EXPECT_TRUE(trimmedEmpty.empty());
    EXPECT_EQ(trimmedEmpty.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, TrimmedPurWhitespaceBecomesEmpty) {
    std::u8string_view trimmedPureWhitespace = StringHelper::GetTrimmed(u8" \t \t ");
    EXPECT_TRUE(trimmedPureWhitespace.empty());
    EXPECT_EQ(trimmedPureWhitespace.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, UntrimmableStringRemainsUnchanged) {
    std::u8string_view untrimmed(u8"x \t\n y");
    std::u8string_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_EQ(untrimmed, trimmed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, CanTrimUtf8StringEndingWithMultiCharacterCodePoint) {
    std::u8string_view untrimmed(u8"M𐍈𐍈  ");
    std::u8string_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_EQ(trimmed, std::u8string_view(u8"M𐍈𐍈"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, CanTrimUtf16StringEndingWithMultiCharacterCodePoint) {
    std::wstring_view untrimmed(L" 😄😄 ");
    std::wstring_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_EQ(trimmed, std::wstring_view(L"😄😄"));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
