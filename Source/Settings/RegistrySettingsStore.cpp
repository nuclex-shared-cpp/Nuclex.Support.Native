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

  bool RegistrySettingsStore::DeleteKey(const std::string &registryPath) {
    return false; // TODO: Implement delete method
  }

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::RegistrySettingsStore(
    const std::string &registryPath, bool readOnly /* = false */
  ) : settingsKeyHandle(0) {
      
    // Attempt to open the requested key
    ::LSTATUS result;
    {
      // Figure out the security flags for the access level requested by the caller
      ::REGSAM securityAccessMethods = (readOnly) ?
        (KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE) :
        (KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE);

      // If no slashes are in the path, it may still be a valid registry hive...
      std::string::size_type firstSlashIndex = findNextSlash(registryPath);
      if(firstSlashIndex == std::string::npos) {
        ::HKEY hiveKeyHandle = hiveFromPrefix(registryPath, registryPath.length());
        result = ::RegOpenKeyExW(
          hiveKeyHandle, nullptr,
          0,
          securityAccessMethods,
          reinterpret_cast<::HKEY *>(&this->settingsKeyHandle)
        );
      } else { // Slashes present, separate the registry hive from the rest
        ::HKEY hiveKeyHandle = hiveFromPrefix(registryPath, firstSlashIndex);

        // We need an UTF-16 string containing the subkey to open, also using only
        // backward slashes (unlike Windows' file system API, the registry API isn't lax)
        std::wstring subkey;
        {
          std::string subkeyUtf8 = registryPath.substr(firstSlashIndex + 1);
          makeAllSlashesBackward(subkeyUtf8);
          subkey = Text::StringConverter::WideFromUtf8(subkeyUtf8);
        }

        // If the key doesn't exist, we do one of two things:
        //
        // - in read-only mode, we act as if an empty key existed. This is consistent with
        //   the behavior of the Retrieve() method and allows applications to start without
        //   their registry keys present, using default settings.
        //
        // - in writable mode, we try to create a new key. This is the correct behavior here
        //   because the caller expects us to hold onto any settings written to the registry
        //   and we can't very well do that if we don't have a registry to write to.
        //
        if(readOnly) {
          result = ::RegOpenKeyExW(
            hiveKeyHandle, subkey.c_str(),
            0,
            securityAccessMethods,
            reinterpret_cast<::HKEY *>(&this->settingsKeyHandle)
          );
          if(result == ERROR_FILE_NOT_FOUND) {
            this->settingsKeyHandle = 0; // RegOpenKeyExW() may write INVALID_HANDLE_VALUe here
            result = ERROR_SUCCESS;
          }
        } else {
          result = ::RegCreateKeyExW(
            hiveKeyHandle, subkey.c_str(),
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            securityAccessMethods,
            nullptr,
            reinterpret_cast<::HKEY *>(&this->settingsKeyHandle),
            nullptr
          );
        } // if read-only
      } // if slashes present
    } // LSTATUS scope

    // Check if the key was opened successfully
    if(result != ERROR_SUCCESS) {
      this->settingsKeyHandle = 0; // RegOpenKeyExW() may write INVALID_HANDLE_VALUe here
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not open registry key", result
      );
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
    if(this->settingsKeyHandle == 0) {
      return std::vector<std::string>(); // Non-existent key accessed in read-only mode
    }

    // Query the number of subkeys in our root settings key
    DWORD subKeyCount;
    DWORD longestSubKeyLength;
    {
      ::LSTATUS result = ::RegQueryInfoKeyW(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
        nullptr, nullptr, nullptr, &subKeyCount,
        &longestSubKeyLength, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
      );
      if(result != ERROR_SUCCESS) {
        Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query number of subkeys from registry key", result
        );
      }
    }

    // Collect a list of all subkeys below the root settings key
    std::vector<std::string> results;
    results.reserve(subKeyCount);
    {
      std::vector<WCHAR> keyName(longestSubKeyLength, 0);
      DWORD subKeyLength = longestSubKeyLength;

      // This is how subkeys are collected, by querying them one by one. Combined with
      // the API documentation stating that when new keys are inserted, their index is
      // random, this design has a high likelihood of producing garbage results if
      // the registry changes while we're enumerating it.
      for(DWORD index = 0;; ++index) {

        // Query the name of the current key. We should have enough buffer size for any
        // subkey present, but the registry can change at any moment, so we'll repeat
        // the query with larger and larger buffer sizes if it fails with ERROR_MORE_DATA
        ::LSTATUS result;
        for(;;) {
          result = ::RegEnumKeyExW(
            *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
            index,
            keyName.data(),
            &subKeyLength,
            nullptr,
            nullptr,
            nullptr,
            nullptr
          );
          if(result == ERROR_MORE_DATA) {
            longestSubKeyLength += 256;
            keyName.resize(longestSubKeyLength);
            subKeyLength = longestSubKeyLength;
          } else {
            break;
          }
        }
        if(result == ERROR_NO_MORE_ITEMS) {
          break; // end reached
        } else if(result != ERROR_SUCCESS) {
          Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not query name of subkey from registry key", result
          );
        }

        // TODO: Instead of Utf8FromWide, manually do the conversion here to avoid a copy

        // The registry API is, like any Windows API, bogged down with Microsoft's
        // poor choice of using UTF-16. So we need to convert everything returned by
        // said method into UTF-8 ourselves.
        results.push_back(Text::StringConverter::Utf8FromWide(keyName.data()));
      }
    }

    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    if(this->settingsKeyHandle == 0) {
      return std::vector<std::string>(); // Non-existent key accessed in read-only mode
    }

    // Query the number of subkeys in our root settings key
    DWORD valueCount;
    DWORD longestValueNameLength;
    {
      ::LSTATUS result = ::RegQueryInfoKeyW(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, &valueCount, &longestValueNameLength,
        nullptr, nullptr, nullptr
      );
      if(result != ERROR_SUCCESS) {
        Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query number of values in registry key", result
        );
      }
    }

    // Collect a list of all subkeys below the root settings key
    std::vector<std::string> results;
    results.reserve(valueCount);
    {
      std::vector<WCHAR> valueName(longestValueNameLength, 0);
      DWORD valueNameLength = longestValueNameLength;

      // This is how values are collected, by querying them one by one. Combined with
      // the API documentation stating that when new keys are inserted, their index is
      // random, this design has a high likelihood of producing garbage results if
      // the registry changes while we're enumerating it.
      for(DWORD index = 0;; ++index) {

        // Query the name of the current key. We should have enough buffer size for any
        // subkey present, but the registry can change at any moment, so we'll repeat
        // the query with larger and larger buffer sizes if it fails with ERROR_MORE_DATA
        ::LSTATUS result;
        for(;;) {
          result = ::RegEnumValueW(
            *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
            index,
            valueName.data(),
            &valueNameLength,
            nullptr,
            nullptr,
            nullptr,
            nullptr
          );
          if(result == ERROR_MORE_DATA) {
            longestValueNameLength += 256;
            valueName.resize(longestValueNameLength);
            valueNameLength = longestValueNameLength;
          } else {
            break;
          }
        }
        if(result == ERROR_NO_MORE_ITEMS) {
          break; // end reached
        } else if(result != ERROR_SUCCESS) {
          Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not query name of subkey from registry key", result
          );
        }

        // TODO: Instead of Utf8FromWide, manually do the conversion here to avoid a copy

        // The registry API is, like any Windows API, bogged down with Microsoft's
        // poor choice of using UTF-16. So we need to convert everything returned by
        // said method into UTF-8 ourselves.
        results.push_back(Text::StringConverter::Utf8FromWide(valueName.data()));
      }
    }

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
