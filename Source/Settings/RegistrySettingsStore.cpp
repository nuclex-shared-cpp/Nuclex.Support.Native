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

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Text/StringMatcher.h"
#include "Nuclex/Support/Text/StringConverter.h"
#include "../Helpers/WindowsApi.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Figures out the registry hive specified in a registry path</summary>
  /// <param name="pathWithPrefix">Full registry path with hive prefix</param>
  /// <param name="prefixLength">Length of the hive prefix in bytes</param>
  /// <returns>The registry hive specified in the registry path</returns>
  /// <remarks>
  ///   This supports both the short form (HKCU/, HKLM/) and the long form
  ///   (HKEY_CURRENT_USER/, HKEY_LOCAL_MACHINE/) for specifying the hive.
  /// </remarks>
  ::HKEY hiveFromPrefix(const std::string &pathWithPrefix, std::size_t prefixLength) {
    using Nuclex::Support::Text::StringMatcher;

    // Check for the short-form hive constants (HKCU, HKLM, etc.)
    if(prefixLength >= 3) {
      bool isHk = (
        ((pathWithPrefix[0] == 'H') || (pathWithPrefix[0] == 'h')) &&
        ((pathWithPrefix[1] == 'K') || (pathWithPrefix[1] == 'k'))
      );
      if(isHk) {

        // Is the prefix 'HKU' for HKEY_USERS?
        if(prefixLength == 3) {
          if((pathWithPrefix[2] == 'U') || (pathWithPrefix[2] == 'u')) {
            return HKEY_USERS;
          }
        }

        if(prefixLength == 4) {

          // Is the prefix 'HKCR', 'HKCU' or 'HKCC'?
          if((pathWithPrefix[2] == 'C') || (pathWithPrefix[2] == 'c')) {
            if((pathWithPrefix[3] == 'R') || (pathWithPrefix[3] == 'r')) {
              return HKEY_CLASSES_ROOT;
            }
            if((pathWithPrefix[3] == 'U') || (pathWithPrefix[3] == 'u')) {
              return HKEY_CURRENT_USER;
            }
            if((pathWithPrefix[3] == 'C') || (pathWithPrefix[3] == 'c')) {
              return HKEY_CURRENT_CONFIG;
            }
          }

          // Is the prefix 'HKLM' for HKEY_LOCAL_MACHINE?
          bool isHklm = (
            ((pathWithPrefix[2] == 'L') || (pathWithPrefix[2] == 'l')) &&
            ((pathWithPrefix[3] == 'M') || (pathWithPrefix[3] == 'm'))
          );
          if(isHklm) {
            return HKEY_LOCAL_MACHINE;
          }

        } // prefixLength == 4
      } // isHk
    } // prefixLength >= 3

    if(prefixLength == 10) {
      if(StringMatcher::StartsWith(pathWithPrefix, u8"HKEY_USERS")) {
        return HKEY_USERS;
      }
    }
    if(prefixLength == 17) {
      if(StringMatcher::StartsWith(pathWithPrefix, u8"HKEY_CLASSES_ROOT")) {
        return HKEY_CLASSES_ROOT;
      }
      if(StringMatcher::StartsWith(pathWithPrefix, u8"HKEY_CURRENT_USER")) {
        return HKEY_CURRENT_USER;
      }
    }
    if(prefixLength == 18) {
      if(StringMatcher::StartsWith(pathWithPrefix, u8"HKEY_LOCAL_MACHINE")) {
        return HKEY_LOCAL_MACHINE;
      }
    }
    if(prefixLength == 19) {
      if(StringMatcher::StartsWith(pathWithPrefix, u8"HKEY_CURRENT_CONFIG")) {
        return HKEY_CURRENT_CONFIG;
      }
    }

    throw std::runtime_error(
      u8"Registry path does not begin with a known registry hive (i.e. HKLM/some/path)"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Looks for the next forward or backward slash in a string</summary>
  /// <param name="path">Path in which the next forward or backward slash is searched</param>
  /// <param nam=e"startIndex">Index at which the search will start</param>
  /// <returns>
  ///   The index of the next forward or backward slash. If no slashes were found,
  ///   std::string::npos is returned.
  /// </returns>
  std::string::size_type findNextSlash(
    const std::string &path, std::string::size_type startIndex = 0
  ) {
    std::string::size_type length = path.length();
    for(std::string::size_type index = startIndex; index < length; ++index) {
      char current = path[index];
      if((current == '\\') || (current == '/')) {
        return index;
      }
    }

    return std::string::npos;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Changes all slashes in a UTF-8 string to backward slashes</summary>
  /// <param name="stringToChange">String in which the slashes will be changed</param>
  void makeAllSlashesBackward(std::string &stringToChange) {
    std::string::size_type length = stringToChange.length();
    for(std::string::size_type index = 0; index < length; ++index) {
      if(stringToChange[index] == '/') {
        stringToChange[index] = '\\';
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  struct PrivateImplementationData {
    public: ::HKEY openedKeyHandle;
  };

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::RegistrySettingsStore(
    const std::string &registryPath, bool readOnly /* = false */
  ) : settingsKeyHandle(0) {
    ::REGSAM securityAccessMethods = (readOnly) ?
      (KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE) :
      (KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE);
      
    std::string::size_type firstSlashIndex = findNextSlash(registryPath);
    if(firstSlashIndex == std::string::npos) {
      ::HKEY hiveKeyHandle = hiveFromPrefix(registryPath, registryPath.length());
      ::LSTATUS result = ::RegOpenKeyExW(
        hiveKeyHandle, nullptr,
        0,
        securityAccessMethods,
        reinterpret_cast<::HKEY *>(&this->settingsKeyHandle)
      );
      if(result != ERROR_SUCCESS) {
        this->settingsKeyHandle = 0; // RegOpenKeyExW() may write INVALID_HANDLE_VALUe here
        Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not open registry key", result
        );
      }
    } else {
      ::HKEY hiveKeyHandle = hiveFromPrefix(registryPath, firstSlashIndex);
      std::wstring subkey;
      {
        std::string subkeyUtf8 = registryPath.substr(firstSlashIndex + 1);
        makeAllSlashesBackward(subkeyUtf8);
        subkey = Text::StringConverter::WideFromUtf8(subkeyUtf8);
      }
      ::LSTATUS result = ::RegOpenKeyExW(
        hiveKeyHandle, subkey.c_str(),
        0,
        securityAccessMethods,
        reinterpret_cast<::HKEY *>(&this->settingsKeyHandle)
      );
      if(result != ERROR_SUCCESS) {
        this->settingsKeyHandle = 0; // RegOpenKeyExW() may write INVALID_HANDLE_VALUe here
        Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not open registry key", result
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::~RegistrySettingsStore() {
    ::HKEY &thisSettingsKeyHandle = *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle != nullptr) {
      ::LSTATUS result = ::RegCloseKey(thisSettingsKeyHandle);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == ERROR_SUCCESS) && u8"Accessed registry key was closed successfully");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllCategories() const {
    std::vector<std::string> results;
    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    std::vector<std::string> results;
    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  bool RegistrySettingsStore::DeleteCategory(const std::string &categoryName) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  bool RegistrySettingsStore::DeleteProperty(
    const std::string &categoryName, const std::string &propertyName
  ) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<bool> RegistrySettingsStore::RetrieveBooleanProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<bool>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint32_t> RegistrySettingsStore::RetrieveUInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::uint32_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int32_t> RegistrySettingsStore::RetrieveInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::int32_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint64_t> RegistrySettingsStore::RetrieveUInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::uint64_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int64_t> RegistrySettingsStore::RetrieveInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::int64_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> RegistrySettingsStore::RetrieveStringProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::string>();
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreBooleanProperty(
    const std::string &categoryName, const std::string &propertyName, bool value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreUInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::uint32_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::int32_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreUInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::uint64_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::int64_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreStringProperty(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // defined(NUCLEX_SUPPORT_WIN32)
