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

}}} // namespace Nuclex::Support::Text
