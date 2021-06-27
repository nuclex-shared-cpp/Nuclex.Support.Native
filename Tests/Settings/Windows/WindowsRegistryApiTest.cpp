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

#include "../Source/Settings/Windows/WindowsRegistryApi.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Settings { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInShortForm) {
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkcr"));
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKCR"));

    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkcc"));
    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKCC"));

    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkcu"));
    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKCU"));

    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hklm"));
    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKLM"));

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hku"));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKU"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInLongForm) {
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkey_classes_root"));
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CLASSES_ROOT"));

    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_config")
    );
    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_CONFIG")
    );

    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_user"));
    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_USER"));

    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hkey_local_machine")
    );
    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKEY_LOCAL_MACHINE")
    );

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hkey_users"));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKEY_USERS"));
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Settings::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)