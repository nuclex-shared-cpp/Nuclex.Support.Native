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

#include "Nuclex/Support/Text/StringMatcher.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StringComparisonDefaultsToCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::AreEqual(u8"Hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual<false>(u8"Hello", u8"hello"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StringComparisonCanBeCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::AreEqual<false>(u8"Hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual<false>(u8"hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual<false>(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_TRUE(StringMatcher::AreEqual<false>(u8"ünicøde", u8"ünicøde"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StringComparisonCanBeCaseSensitive) {
    EXPECT_FALSE(StringMatcher::AreEqual<true>(u8"Hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual<true>(u8"hello", u8"hello"));
    EXPECT_FALSE(StringMatcher::AreEqual<true>(u8"Ünicøde", u8"ünicØde"));
    EXPECT_FALSE(StringMatcher::AreEqual<true>(u8"ÜNICØDE", u8"üNICøDE"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckForContainmentCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"Hello World", u8"hello"));
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"Hello World", u8"world"));

    EXPECT_TRUE(StringMatcher::Contains<false>(u8"HellØ WØrld", u8"hellø"));
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"HellØ WØrld", u8"wørld"));

    EXPECT_TRUE(StringMatcher::Contains<false>(u8"Hello World", u8"h"));
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"Hello World", u8"w"));

    EXPECT_FALSE(StringMatcher::Contains<false>(u8"H", u8"hello"));
    EXPECT_FALSE(StringMatcher::Contains<false>(u8"W", u8"world"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, ContainmentCheckHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"Hello World", u8""));
    EXPECT_TRUE(StringMatcher::Contains<false>(u8"", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckForContainmentCaseSensitive) {
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"Hello World", u8"Hello"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"Hello World", u8"hello"));
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"Hello World", u8"World"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"Hello World", u8"world"));

    EXPECT_TRUE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"HellØ"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"hellø"));
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"WØrld"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"wørld"));

    EXPECT_TRUE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"H"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"h"));
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"W"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"HellØ WØrld", u8"w"));

    EXPECT_FALSE(StringMatcher::Contains<true>(u8"H", u8"Hello"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"H", u8"hello"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"W", u8"World"));
    EXPECT_FALSE(StringMatcher::Contains<true>(u8"W", u8"world"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, ContainmentCheckHandlesEmptyNeedleCaseSensitive) {
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"Hello World", u8""));
    EXPECT_TRUE(StringMatcher::Contains<true>(u8"", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringStartsWithAnotherCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith<false>(u8"Hello World", u8"Hello"));
    EXPECT_TRUE(StringMatcher::StartsWith<false>(u8"Hello World", u8"hello"));
    EXPECT_FALSE(StringMatcher::StartsWith<false>(u8"Hello World", u8"World"));

    EXPECT_TRUE(StringMatcher::StartsWith<false>(u8"HellØ WØrld", u8"HellØ"));
    EXPECT_TRUE(StringMatcher::StartsWith<false>(u8"HellØ WØrld", u8"hellø"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StartsWithHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith<false>(u8"Hello World", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringStartsWithAnotherCaseSensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith<true>(u8"Hello World", u8"Hello"));
    EXPECT_FALSE(StringMatcher::StartsWith<true>(u8"Hello World", u8"hello"));
    EXPECT_FALSE(StringMatcher::StartsWith<true>(u8"Hello World", u8"World"));

    EXPECT_TRUE(StringMatcher::StartsWith<true>(u8"HellØ WØrld", u8"HellØ"));
    EXPECT_FALSE(StringMatcher::StartsWith<true>(u8"HellØ WØrld", u8"hellø"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StartsWithHandlesEmptyNeedleCaseSensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith<true>(u8"Hello World", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringEndsWithAnotherCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::EndsWith<false>(u8"Hello World", u8"World"));
    EXPECT_TRUE(StringMatcher::EndsWith<false>(u8"Hello World", u8"world"));
    EXPECT_FALSE(StringMatcher::EndsWith<false>(u8"Hello World", u8"Hello"));

    EXPECT_TRUE(StringMatcher::EndsWith<false>(u8"HellØ WØrld", u8"WØrld"));
    EXPECT_TRUE(StringMatcher::EndsWith<false>(u8"HellØ WØrld", u8"wørld"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, EndsWithHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::EndsWith<false>(u8"Hello World", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringEndsWithAnotherCaseSensitive) {
    EXPECT_TRUE(StringMatcher::EndsWith<true>(u8"Hello World", u8"World"));
    EXPECT_FALSE(StringMatcher::EndsWith<true>(u8"Hello World", u8"world"));
    EXPECT_FALSE(StringMatcher::EndsWith<true>(u8"Hello World", u8"Hello"));

    EXPECT_TRUE(StringMatcher::EndsWith<true>(u8"HellØ WØrld", u8"WØrld"));
    EXPECT_FALSE(StringMatcher::EndsWith<true>(u8"HellØ WØrld", u8"wørld"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, EndsWithHandlesEmptyNeedleCaseSensitive) {
    EXPECT_TRUE(StringMatcher::EndsWith<true>(u8"Hello World", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, EndsWithDetectsStartInsideCodepoint) {
    EXPECT_FALSE(StringMatcher::EndsWith<true>(u8"Hello Ɯorld", u8"world"));
    EXPECT_FALSE(StringMatcher::EndsWith<false>(u8"Hello Ɯorld", u8"world"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, WilcardMatchDefaultsToCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"Hello World", u8"hello*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard<false>(u8"HellØ WØrld", u8"hellø*"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, WilcardMatchCanBeCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::FitsWildcard<false>(u8"Hello World", u8"hello world"));
    EXPECT_TRUE(StringMatcher::FitsWildcard<false>(u8"HellØ WØrld", u8"hellø wørld"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, WildcardMatchCanBeCaseSensitive) {
    EXPECT_FALSE(StringMatcher::FitsWildcard<true>(u8"Hello World", u8"hello world"));
    EXPECT_FALSE(StringMatcher::FitsWildcard<true>(u8"HellØ WØrld", u8"hellø wørld"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanMatchAsciiStringToWildcard) {
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello World"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", ""));
    EXPECT_TRUE(StringMatcher::FitsWildcard("", ""));
    EXPECT_FALSE(StringMatcher::FitsWildcard("", "Hello World"));

    EXPECT_TRUE(StringMatcher::FitsWildcard("", "*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "He*o World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hell*o World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "W*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*W*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello World*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*Hello World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello***World"));

    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hell? W?rld"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "?Hello World"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "Hello World?"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanMatchUtf8StringToWildcard) {
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"He*ø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hell*ø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"DLRØW ØLLEH", u8"ø*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*ø*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø Wørld*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*Hellø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø***Wørld"));

    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"H?llø Wør?d"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"?Hellø Wørld"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø Wørld?"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringHashWorks) {
    CaseInsensitiveUtf8Hash hasher;
    std::size_t hash1 = hasher(u8"Hellø Wørld This is a test for the hashing method");
    std::size_t hash2 = hasher(u8"Hellø Wørld This is another test for the hashing method");
    std::size_t hash3 = hasher(u8"HELLØ WØRLD This is a test for the hashing method");

    EXPECT_EQ(hash1, hash3);
    EXPECT_NE(hash1, hash2);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringEqualsToWorks) {
    CaseInsensitiveUtf8EqualTo equals;

    EXPECT_TRUE(equals(u8"Hello", u8"hello"));
    EXPECT_TRUE(equals(u8"hello", u8"hello"));
    EXPECT_TRUE(equals(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_TRUE(equals(u8"ünicøde", u8"ünicøde"));
    EXPECT_FALSE(equals(u8"hello", u8"olleh"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringLessWorks) {
    CaseInsensitiveUtf8Less lesser;

    EXPECT_TRUE(lesser(u8"a", u8"b"));
    EXPECT_FALSE(lesser(u8"b", u8"b"));
    EXPECT_TRUE(lesser(u8"a9999", u8"b0000"));
    EXPECT_TRUE(lesser(u8"a9999", u8"b0"));
    EXPECT_TRUE(lesser(u8"a", u8"aa"));

    // Neither is less because in lowercase, they're identical
    EXPECT_FALSE(lesser(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_FALSE(lesser(u8"üNICØDE", u8"Ünicøde"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanFindSubstringCaseInsensitive) {
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"hello"), 0);
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"world"), 6);
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"o w"), 4);
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"world!"), std::string::npos);
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"Hello World"), 0);
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8"Hello World!"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, FindHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_EQ(StringMatcher::Find<false>(u8"Hello World", u8""), 0);
    EXPECT_EQ(StringMatcher::Find<false>(u8"", u8""), 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanFindSubstringCaseSensitive) {
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"Hello"), 0);
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"World"), 6);
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"o W"), 4);
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"world"), std::string::npos);
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"Hello World"), 0);
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8"Hello World!"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, FindHandlesEmptyNeedleCaseSensitive) {
    EXPECT_EQ(StringMatcher::Find<true>(u8"Hello World", u8""), 0);
    EXPECT_EQ(StringMatcher::Find<true>(u8"", u8""), 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanFindUtf8SubstringCaseInsensitive) {
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"hellø"), 0);
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"wørld"), 7);
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"ø w"), 4);
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"wørld!"), std::string::npos);
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"HellØ WØrld"), 0);
    EXPECT_EQ(StringMatcher::Find<false>(u8"HellØ WØrld", u8"HellØ WØrld!"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanFindUtf8SubstringCaseSensitive) {
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"HellØ"), 0);
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"WØrld"), 7);
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"ø W"), std::string::npos);
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"wørld"), std::string::npos);
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"HellØ WØrld"), 0);
    EXPECT_EQ(StringMatcher::Find<true>(u8"HellØ WØrld", u8"HellØ WØrld!"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
