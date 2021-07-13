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

#include "Nuclex/Support/Text/StringMatcher.h"
#include "Nuclex/Support/Text/StringConverter.h"

#include "../Platform/WindowsRegistryApi.h"

#include <cassert> // for assert()

namespace {

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
        ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
          registryPath, registryPath.length()
        );
        result = ::RegOpenKeyExW(
          hiveKeyHandle, nullptr,
          0,
          securityAccessMethods,
          reinterpret_cast<::HKEY *>(&this->settingsKeyHandle)
        );
      } else { // Slashes present, separate the registry hive from the rest
        ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
          registryPath, firstSlashIndex
        );

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
      Platform::WindowsApi::ThrowExceptionForSystemError(
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

    return Platform::WindowsRegistryApi::GetAllSubKeyNames(
      *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    if(this->settingsKeyHandle == 0) {
      return std::vector<std::string>(); // Non-existent key accessed in read-only mode
    }

    return Platform::WindowsRegistryApi::GetAllValueNames(
      *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle)
    );
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
    if(this->settingsKeyHandle == 0) {
      return std::optional<bool>();
    }


/*
    //DWORD 
    ::RegQueryValueExW(
      this->settingsKeyHandle
    )
*/
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

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
