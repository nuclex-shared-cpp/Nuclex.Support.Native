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

#include "Nuclex/Support/Settings/RegistrySettingsStore.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>
#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanOpenHiveInShortForm) {
    EXPECT_NO_THROW(
      RegistrySettingsStore settings(u8"hkcu");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanOpenHiveInLongForm) {
    EXPECT_NO_THROW(
      RegistrySettingsStore settings(u8"HKEY_CLASSES_ROOT");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, ThrowsExceptionWhenNoHiveSpecified) {
    EXPECT_THROW(
      RegistrySettingsStore settings(u8"SOFTWARE/Microsoft"),
      std::runtime_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanAccessDeepRegistryKey) {
    RegistrySettingsStore settings(u8"HKEY_CURRENT_USER/SOFTWARE/Microsoft");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanAccessPrivilegedKeyReadOnly) {
    const RegistrySettingsStore settings(u8"HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft", true);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, NonExistentKeyCanBeAccessedInReadOnlyMode) {
    // If a non-existent key is specified in read-only mode, the settings store acts
    // as if it was completely empty. This makes the behavior consistent with the Retrieeve()
    // method. If an error was thrown instead, it would make applications un-runnable unless
    // some useless, empty registry was present rather than gracefully using default settings.
    EXPECT_NO_THROW(
      const RegistrySettingsStore settings(u8"HKEY_LOCAL_MACHINE/Lalala123ThisDoesntExist", true);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, AttemptsCreationOfNonExistentKey) {
    // The behavior is different in writable mode. The user expects to be able to store
    // settings in the registry, so if the key doesn't exist, it's immediately created
    // and when that isn't possible (bad path or privilege issue), an error gets thrown.
    //
    // I hope you're not running your unit test with administrative privileges...
    EXPECT_THROW(
      const RegistrySettingsStore settings(u8"HKEY_LOCAL_MACHINE/Lalala123ThisDoesntExist"),
      std::system_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanEnumerateCategories) {
    const RegistrySettingsStore settings(u8"hklm/SOFTWARE/Microsoft", true);

    std::vector<std::string> categories = settings.GetAllCategories();
    EXPECT_GE(categories.size(), 10U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RegistrySettingsStoreTest, CanEnumerateProperties) {
    const RegistrySettingsStore settings(u8"HKLM/SYSTEM/CurrentControlSet/Control", true);

    std::vector<std::string> properties = settings.GetAllProperties();
    EXPECT_GE(properties.size(), 5U);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
