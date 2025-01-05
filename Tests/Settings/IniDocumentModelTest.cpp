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

#include "../../Source/Settings/IniDocumentModel.h"

#include <gtest/gtest.h>

// We allow this now.
#define ALLOW_NEWLINES_IN_QUOTED_STRINGS 1

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An average .ini file without any special or ambiguous contents</summary>
  const char VanillaIniFile[] =
    u8"GlobalProperty=1\n"
    u8"\n"
    u8"[ImportantStuff]\n"
    u8";CommentedOut=5000\n"
    u8"Normal=42\n"
    u8"\n";

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An .ini file with empty assignments and a padded section</summary>
  const char EmptyAssignments[] =
    u8"WithoutValue=\n"
    u8"\n"
    u8"[ MoreStuff ]\n"
    u8"AlsoNoValue = ;\n"
    u8"TrailingSpaces = Hello  \n"
    u8"Quoted = \"Hello \" \n"
    u8"WeirdOne = \"\n"
    u8"YetAgain = #";

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An .ini file with lots of corner cases and malformed statements</summary>
  const char MalformedLines[] =
    u8"ThisLineIsMeaningless\n"
    u8"\n"
    u8"]BadLine1=123\n"
    u8"\"BadLine2=234\"\n"
    u8"[NotASection]=345\n"
    u8"[AlsoNoSection]=[Value]\n"
    u8"Funny = [Hello] [World]\n"
    u8"\n"
    u8"[BadLine3 = 456]\n"
    u8"BadLine4 = 567 = 789\n"
    u8"\"Bad\" Line5=890\n"
    u8"Bad \"Line6\"=1\n"
    u8"\n"
    u8"[\"Quoted Section\"]\n"
    u8"[\"BadSection]\"\n"
    u8"GoodLine=2 3\n"
    u8"BadLine7=\"4\" 5\n"
    u8"BadLine7=6 \"7\"";

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An .ini file with quoted strings continuing into the next line</summary>
  const char MultilineStrings[] =
    u8"Multiline = \"\n"
    u8"  Hello World\n"
    u8"\"\n"
    u8"[Section]\n"
    u8"MultilineWithComment = \"Hello # World\n"
    u8"# Again\"\n";

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, DefaultConstructedModelHasNoSections) {
    IniDocumentModel dom;
    std::vector<std::string> sections = dom.GetAllSections();
    EXPECT_EQ(sections.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasFileContentsConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::byte *>(VanillaIniFile),
        sizeof(VanillaIniFile) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, EmptyDocumentCanBeSerialized) {
    IniDocumentModel dom;

    std::vector<std::byte> contents = dom.Serialize();
    EXPECT_EQ(contents.size(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanParseVanillaProperty) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"GlobalProperty");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"1");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanParseVanillaSection) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"ImportantStuff", u8"Normal");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"42");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, NamesAreCaseInsensitive) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"impOrtantstUff", u8"nOrmAl");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"42");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, IgnoresComments) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"ImportantStuff", u8"CommentedOut");
    EXPECT_FALSE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanHandleEmptyAssignments) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::byte *>(EmptyAssignments),
        sizeof(EmptyAssignments) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, AssignmentWithoutValueIsValid) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"WithoutValue");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), std::string());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SectionCanBePaddedWithSpaces) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"AlsoNoValue");
    EXPECT_TRUE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CommentAfterPropertyValueIsOmitted) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );

    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"AlsoNoValue");
    ASSERT_TRUE(value.has_value());
    EXPECT_STREQ(value.value().c_str(), u8"");

    value = dom.GetPropertyValue(u8"MoreStuff", u8"YetAgain");
#if defined(ALLOW_NEWLINES_IN_QUOTED_STRINGS)
    EXPECT_FALSE(value.has_value());
#else
    ASSERT_TRUE(value.has_value());
    EXPECT_STREQ(value.value().c_str(), u8"");
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SpacesAfterPropertyValueAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"TrailingSpaces");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"Hello");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SpacesInsideQuotesAreKept) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"Quoted");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"Hello ");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, UnclosedQuoteInvalidatesLine) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(EmptyAssignments),
      sizeof(EmptyAssignments) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(u8"MoreStuff", u8"WeirdOne");

    // If allowing multi-line strings, the quote isn't being closed, so it's malformed
    EXPECT_FALSE(value.has_value());

    // We can still check if the opened quote "ate" the next line (in case multi-line
    // quoted strings are allowed) or if the next line was parsed on its own.
#if defined(ALLOW_NEWLINES_IN_QUOTED_STRINGS)
    EXPECT_FALSE(dom.GetPropertyValue(u8"MoreStuff", u8"YetAgain").has_value());
#else
    EXPECT_TRUE(dom.GetPropertyValue(u8"MoreStuff", u8"YetAgain").has_value());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, CanHandleMalformedLines) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::byte *>(MalformedLines),
        sizeof(MalformedLines) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, MalformedLinesAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );

    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"ThisLineIsMeaningless").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"]BadLine1").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"BadLine1").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"BadLine2").has_value()
    );
    EXPECT_FALSE(
      dom.GetPropertyValue(std::string(), u8"\"BadLine2").has_value()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, SectionNameCanHaveQuotes) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::vector<std::string> sections = dom.GetAllSections();
    bool found = false;
    for(std::size_t index = 0; index < sections.size(); ++index) {
      if(sections[index] == u8"Quoted Section") {
        found = true;
      }
    }
    EXPECT_TRUE(found);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyNameCanHaveBrackets) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"NotASection");
    EXPECT_TRUE(value.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyNameAndValueCanHaveBrackets) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );
    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"AlsoNoSection");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), u8"[Value]");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, AllMalformedElementsAreIgnored) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MalformedLines),
      sizeof(MalformedLines) - 1
    );

    // All "bad" (malformed) sections and lines conveniently have a name
    // that includes the word "Bad" :-)
    bool badSectionOrPropertyFound = false;

    std::vector<std::string> sections = dom.GetAllSections();
    for(std::size_t index = 0; index < sections.size(); ++index) {
      if(sections[index].find("Bad") != std::string::npos) {
        badSectionOrPropertyFound = true;
      }
    }
    for(std::size_t sectionIndex = 0; sectionIndex < sections.size(); ++sectionIndex) {
      std::vector<std::string> properties = dom.GetAllProperties(sections[sectionIndex]);
      for(std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex) {
        if(properties[propertyIndex].find("Bad") != std::string::npos) {
          badSectionOrPropertyFound = true;
        }
      }
    }

    EXPECT_FALSE(badSectionOrPropertyFound);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, NewPropertiesCanBeCreated) {
    IniDocumentModel dom;
    dom.SetPropertyValue(u8"MySection", u8"World", u8"Hello");
    dom.SetPropertyValue(std::string(), u8"Hello", u8"World");

    std::vector<std::byte> fileContents = dom.Serialize();

    std::string fileContentsAsString(
      reinterpret_cast<const char *>(fileContents.data()), fileContents.size()
    );
    EXPECT_TRUE(fileContentsAsString.find(u8"Hello = World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyValueCanBeChangedToShorter) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    dom.SetPropertyValue(u8"ImportantStuff", u8"Normal", u8"2");

    std::vector<std::byte> fileContents = dom.Serialize();

    std::string fileContentsAsString(
      reinterpret_cast<const char *>(fileContents.data()), fileContents.size()
    );
    EXPECT_TRUE(fileContentsAsString.find(u8"Normal=2\n") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, PropertyValueCanBeChangedToLonger) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(VanillaIniFile),
      sizeof(VanillaIniFile) - 1
    );
    dom.SetPropertyValue(u8"ImportantStuff", u8"Normal", u8"Crazy");

    std::vector<std::byte> fileContents = dom.Serialize();

    std::string fileContentsAsString(
      reinterpret_cast<const char *>(fileContents.data()), fileContents.size()
    );
    EXPECT_TRUE(fileContentsAsString.find(u8"Normal=Crazy\n") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, QuotedStringsCanContainLineBreaks) {
    IniDocumentModel dom(
      reinterpret_cast<const std::byte *>(MultilineStrings),
      sizeof(MultilineStrings) - 1
    );

    std::optional<std::string> value = dom.GetPropertyValue(std::string(), u8"Multiline");
#if defined(ALLOW_NEWLINES_IN_QUOTED_STRINGS)
    ASSERT_TRUE(value.has_value());
    EXPECT_STREQ(value.value().c_str(), u8"\n  Hello World\n");
#else
    EXPECT_FALSE(value.has_value());
#endif

    value = dom.GetPropertyValue(u8"Section", u8"MultilineWithComment");
#if defined(ALLOW_NEWLINES_IN_QUOTED_STRINGS)
    ASSERT_TRUE(value.has_value());
    EXPECT_STREQ(value.value().c_str(), u8"Hello # World\n# Again");
#else
    EXPECT_FALSE(value.has_value());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, EqualsSignCanBeUsedInValue) {
    std::vector<std::byte> serialized;
    {
      IniDocumentModel dom;
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"123");
      dom.SetPropertyValue(u8"Section", u8"Option", u8"Property=Value");
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"New=Value");
      serialized = dom.Serialize();
    }
    {
      IniDocumentModel dom(serialized.data(), serialized.size());

      std::optional<std::string> value = dom.GetPropertyValue(u8"Section", u8"Option");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"Property=Value");

      value = dom.GetPropertyValue(u8"Section", u8"ChangedOption");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"New=Value");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, BackslashCanBeUsedInValue) {
    std::vector<std::byte> serialized;
    {
      IniDocumentModel dom;
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"123");
      dom.SetPropertyValue(u8"Section", u8"Option", u8"Property\\Value");
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"New\\Value");
      serialized = dom.Serialize();
    }
    {
      IniDocumentModel dom(serialized.data(), serialized.size());

      std::optional<std::string> value = dom.GetPropertyValue(u8"Section", u8"Option");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"Property\\Value");

      value = dom.GetPropertyValue(u8"Section", u8"ChangedOption");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"New\\Value");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, QuotesCanBeUsedInValue) {
    std::vector<std::byte> serialized;
    {
      IniDocumentModel dom;
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"123");
      dom.SetPropertyValue(u8"Section", u8"Option", u8"Property\"Value");
      dom.SetPropertyValue(u8"Section", u8"ChangedOption", u8"New\"Value");
      serialized = dom.Serialize();
    }
    {
      IniDocumentModel dom(serialized.data(), serialized.size());

      std::optional<std::string> value = dom.GetPropertyValue(u8"Section", u8"Option");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"Property\"Value");

      value = dom.GetPropertyValue(u8"Section", u8"ChangedOption");
      ASSERT_TRUE(value.has_value());
      EXPECT_STREQ(value.value().c_str(), u8"New\"Value");
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
