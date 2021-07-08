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

#include "Nuclex/Support/Settings/IniSettingsStore.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An average .ini file without any special or ambiguous contents</summary>
  const char ExampleIniFile[] =
    u8"NumericBoolean = 1\n"
    u8"TrueFalseBoolean = TRUE\n"
    u8"YesNoBoolean = YES\n"
    u8"\n"
    u8"[Integers]\n"
    u8"Tiny = 42\n"
    u8"Negative = -42\n"
    u8"Big = 1152921504606846976\n"
    u8"BigNegative = -1152921504606846976\n"
    u8"\n"
    u8"[Strings]\n"
    u8"Simple = Hello\n"
    u8"Quoted = \"World\"\n"
    u8"\n";

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      IniSettingsStore store;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanLoadIniFileFromMemory) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadBooleanTypes) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::string none;
    std::optional<bool> numericBoolean = store.Retrieve<bool>(none, u8"NumericBoolean");
    ASSERT_TRUE(numericBoolean.has_value());
    EXPECT_TRUE(numericBoolean.value());

    std::optional<bool> trueFalseBoolean = store.Retrieve<bool>(none, u8"TrueFalseBoolean");
    ASSERT_TRUE(trueFalseBoolean.has_value());
    EXPECT_TRUE(trueFalseBoolean.value());

    std::optional<bool> yesNoBoolean = store.Retrieve<bool>(none, u8"YesNoBoolean");
    ASSERT_TRUE(yesNoBoolean.has_value());
    EXPECT_TRUE(yesNoBoolean.value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadUnsigned32BitIntegers) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::uint32_t> integer = store.Retrieve<std::uint32_t>(
      u8"Integers", u8"Tiny"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 42U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadSigned32BitIntegers) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::int32_t> integer = store.Retrieve<std::int32_t>(
      u8"Integers", u8"Tiny"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 42);

    std::optional<std::int32_t> negativeInteger = store.Retrieve<std::int32_t>(
      u8"Integers", u8"Negative"
    );
    ASSERT_TRUE(negativeInteger.has_value());
    EXPECT_EQ(negativeInteger.value(), -42);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadUnsigned64BitIntegers) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::uint64_t> integer = store.Retrieve<std::uint64_t>(
      u8"Integers", u8"Big"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 1152921504606846976ULL);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadSigned64BitIntegers) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::int64_t> integer = store.Retrieve<std::int64_t>(
      u8"Integers", u8"Big"
    );
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer.value(), 1152921504606846976LL);

    std::optional<std::int64_t> negativeInteger = store.Retrieve<std::int64_t>(
      u8"Integers", u8"BigNegative"
    );
    ASSERT_TRUE(negativeInteger.has_value());
    EXPECT_EQ(negativeInteger.value(), -1152921504606846976LL);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniSettingsStoreTest, CanReadStrings) {
    IniSettingsStore store;
    store.Load(
      reinterpret_cast<const std::uint8_t *>(ExampleIniFile),
      sizeof(ExampleIniFile) - 1
    );

    std::optional<std::string> simpleString = store.Retrieve<std::string>(
      u8"Strings", u8"Simple"
    );
    ASSERT_TRUE(simpleString.has_value());
    EXPECT_EQ(simpleString.value(), u8"Hello");

    std::optional<std::string> quotedString = store.Retrieve<std::string>(
      u8"Strings", u8"Quoted"
    );
    ASSERT_TRUE(quotedString.has_value());
    EXPECT_EQ(quotedString.value(), u8"World");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings