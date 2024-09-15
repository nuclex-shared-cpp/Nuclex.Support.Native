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
    std::string expected(u8"This is a test", 14);

    // The main point of this test is to verify that there are no out-of-bounds
    // accesses (as would happen when reading from the empty substring).
    std::string test = expected;
    StringHelper::EraseSubstrings(test, std::string());

    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleOccurrenceOfSubstringCanBeRemoved) {
    std::string test(u8"This test did not succeed", 25);
    std::string expected(u8"This test did succeed", 21);

    StringHelper::EraseSubstrings(test, u8" not");

    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, WholeStringCanMatchSubstring) {
    std::string test(u8"Test", 4);
    std::string expected(u8"", 0);

    StringHelper::EraseSubstrings(test, u8"Test");

    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, DuplicateWhitespaceCanBeCollapsedWithoutTrim) {
    std::string test(u8" This  is   an example  ", 24);
    std::string expected(u8" This is an example ", 20);

    StringHelper::CollapseDuplicateWhitespace(test, false);

    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, DuplicateWhitespaceCanBeCollapsedWithTrim) {
    std::string test(u8"  This  is   an example ", 24);
    std::string expected(u8"This is an example", 18);

    StringHelper::CollapseDuplicateWhitespace(test, true);

    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, EmptyStringCanBeWhitespaceCollapsed) {
    std::string test(u8"", 0);
    std::string test2(test);
    std::string expected(u8"", 0);

    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_STREQ(test.c_str(), expected.c_str());

    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_STREQ(test2.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleSpaceCanBeWhitespaceCollapsed) {
    std::string test(u8" ", 1);
    std::string test2(test);

    std::string expected(u8" ", 1);
    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_STREQ(test.c_str(), expected.c_str());

    std::string expected2(u8"", 0);
    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_STREQ(test2.c_str(), expected2.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SpacesOnlyCanBeWhitespaceCollapsed) {
    std::string test(u8"   ", 3);
    std::string test2(test);

    std::string expected(u8" ", 1);
    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_STREQ(test.c_str(), expected.c_str());

    std::string expected2(u8"", 0);
    StringHelper::CollapseDuplicateWhitespace(test2, true);
    ASSERT_STREQ(test2.c_str(), expected2.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, StringEndingInWhitespaceCanBeCollapsedWithTrim) {
    std::string test(u8"Hello World ", 12);
    std::string expected(u8"Hello World", 11);

    StringHelper::CollapseDuplicateWhitespace(test, true);
    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, SingleCharacterSurvivesWhitespaceCollapse) {
    std::string test(u8"d", 1);
    std::string expected(u8"d", 1);

    StringHelper::CollapseDuplicateWhitespace(test, false);
    ASSERT_STREQ(test.c_str(), expected.c_str());

    StringHelper::CollapseDuplicateWhitespace(test, true);
    ASSERT_STREQ(test.c_str(), expected.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, TrimmedEmptyStringIsStillEmpty) {
    std::string_view trimmedEmpty = StringHelper::GetTrimmed(std::string());
    EXPECT_TRUE(trimmedEmpty.empty());
    EXPECT_EQ(trimmedEmpty.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, TrimmedPurWhitespaceBecomesEmpty) {
    std::string_view trimmedPureWhitespace = StringHelper::GetTrimmed(u8" \t \t ");
    EXPECT_TRUE(trimmedPureWhitespace.empty());
    EXPECT_EQ(trimmedPureWhitespace.length(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, UntrimmableStringRemainsUnchanged) {
    std::string_view untrimmed(u8"x \t\n y");
    std::string_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_TRUE(untrimmed == trimmed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, CanTrimUtf8StringEndingWithMultiCharacterCodePoint) {
    std::string_view untrimmed(u8"M𐍈𐍈  ");
    std::string_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_TRUE(trimmed == std::string_view(u8"M𐍈𐍈"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringHelperTest, CanTrimUtf16StringEndingWithMultiCharacterCodePoint) {
    std::wstring_view untrimmed(L" 😄😄 ");
    std::wstring_view trimmed = StringHelper::GetTrimmed(untrimmed);
    EXPECT_TRUE(trimmed == std::wstring_view(L"😄😄"));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
