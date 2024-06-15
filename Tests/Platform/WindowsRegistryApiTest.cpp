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

#include "../../Source/Platform/WindowsRegistryApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInShortForm) {
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkcr", 4));
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKCR", 4));

    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkcc", 4));
    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKCC", 4));

    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkcu", 4));
    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKCU", 4));

    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hklm", 4));
    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKLM", 4));

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hku", 3));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKU", 3));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInLongForm) {
    EXPECT_EQ(
      HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkey_classes_root", 17)
    );
    EXPECT_EQ(
      HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CLASSES_ROOT", 17)
    );

    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_config", 19)
    );
    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_CONFIG", 19)
    );

    EXPECT_EQ(
      HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_user", 17)
    );
    EXPECT_EQ(
      HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_USER", 17)
    );

    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hkey_local_machine", 18)
    );
    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKEY_LOCAL_MACHINE", 18)
    );

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hkey_users", 10));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKEY_USERS", 10));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)