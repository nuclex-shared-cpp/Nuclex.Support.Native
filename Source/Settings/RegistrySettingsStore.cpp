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

#include "Nuclex/Support/ScopeGuard.h"
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

  /// <summary>Opens a subkey below the specified parent registry key</summary>
  /// <param name="parentKeyHandle">Handle of the parent registry key</param>
  /// <param name="subKeyName">Name of the subkey that will be opened</param>
  /// <param name="writable">Whether the key will be opened with write permissions</param>
  /// <returns>
  ///   The handle of the opened registry subkey or a null pointer if the key doesn't exist
  /// </returns>
  ::HKEY openExistingSubKey(
    ::HKEY parentKeyHandle, const std::wstring &subKeyName, bool writable = false
  ) {
    ::HKEY subKeyHandle;
    {
      // Flags to tell the "security accounts manager" the access level we need
      ::REGSAM desiredAccessLevel = KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS;
      if(writable) {
        desiredAccessLevel |= (KEY_SET_VALUE | KEY_CREATE_SUB_KEY);
      }

      // Attempts to open the key. This does not create a key.
      LSTATUS result = ::RegOpenKeyExW(
        parentKeyHandle,
        subKeyName.empty() ? nullptr : subKeyName.c_str(),
        0, // options
        desiredAccessLevel,
        &subKeyHandle
      );
      if(unlikely(result != ERROR_SUCCESS)) {
        if(result == ERROR_FILE_NOT_FOUND) {
          return ::HKEY(nullptr);
        } else {
          Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not open registry subkey for reading", result
          );
        }
      }
    }

    return subKeyHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Opens or creates a subkey below the specified parent registry key</summary>
  /// <param name="parentKeyHandle">Handle of the parent registry key</param>
  /// <param name="subKeyName">Name of the subkey that will be opened or created</param>
  /// <returns>The handle of the opened or created registry subkey</returns>
  ::HKEY openOrCreateSubKey(::HKEY parentKeyHandle, const std::wstring &subKeyName) {
    ::HKEY openedSubKey;
    {
      ::LSTATUS result = ::RegCreateKeyExW(
        parentKeyHandle,
        subKeyName.c_str(),
        0, // reserved
        nullptr, // class ("user-defined type of this key" - no clue)
        REG_OPTION_NON_VOLATILE,
        KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
        nullptr, // security attributes
        &openedSubKey,
        nullptr // disposition - tells whether new key was created - we don't care
      );
      if(result != ERROR_SUCCESS) {
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not open or create registry subkey for read/write access", result
        );
      }
    }

    return openedSubKey;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if the data type of a registry value is supported by this class</summary>
  /// <param name="valueType">Registry value type returned by the registry API</param>
  /// <returns>True if this library understands the specified value type</returns>
  bool isValueTypeSupported(::DWORD valueType) {
    return (
      (valueType == REG_DWORD) ||
      (valueType == REG_DWORD_LITTLE_ENDIAN) ||
      (valueType == REG_DWORD_BIG_ENDIAN) ||
      (valueType == REG_QWORD) ||
      (valueType == REG_QWORD_LITTLE_ENDIAN) ||
      (valueType == REG_SZ)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TValue>
  TValue readKeyValue(::HKEY keyHandle, const std::string &valueName) {
    BYTE stackValue[8];

    DWORD valueType;
    DWORD valueSize;
    BYTE *value;
    {
      std::wstring propertyNameUtf16 = Text::StringConverter::WideFromUtf8(propertyName);

      ::LSTATUS result = ::RegQueryValueExW(
        thisSettingsKeyHandle,
        propertyNameUtf16.c_str(),
        nullptr,
        &valueType,
        value,
        &valueSize
      );
      if(status == ERROR_MORE_DATA) {
        std::vector<std::uint8_t> longValue;
        longValue.resize(valueSize);

        result = ::RegQueryValueExW(
          thisSettingsKeyHandle,
          propertyNameUtf16.c_str(),
          nullptr,
          &valueType,
          value,
          &valueSize
        );
      } else if(status != ERROR_SUCCESS) {
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query registry value", result
        );
      }
    }

    switch(valueType) {
      case REG_DWORD: {
        return TValue(*reinterpret_cast<::DWORD *>(value));
      }
      case REG_QWORD: {
        return TValue(reinterpret_cast<::LARGE_INTEGER *>(value)->QuadPart);
      }
      case REG_SZ: {

      }
    }

    //return TValue()

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
      // If no slashes are in the path, it may still be a valid registry hive...
      std::string::size_type firstSlashIndex = findNextSlash(registryPath);
      if(firstSlashIndex == std::string::npos) {
        ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
          registryPath, registryPath.length()
        );
        *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = openExistingSubKey(
          hiveKeyHandle, std::wstring(), !readOnly
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
          *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = (
            openExistingSubKey(hiveKeyHandle, subkey)
          );
        } else {
          *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = (
            openOrCreateSubKey(hiveKeyHandle, subkey)
          );
        } // if read-only
      } // if slashes present
    } // LSTATUS scope
  }

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::~RegistrySettingsStore() {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle);
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
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<bool>();
    }
/*
    if(categoryName.empty()) {
      DWORD valueType;

      std::wstring propertyNameUtf16 = Text::StringConverter::WideFromUtf8(propertyName);
      ::RegQueryValueExW(
        thisSettingsKeyHandle,
        propertyNameUtf16.c_str(),
        nullptr,
        &valueType,


      );
      // Query directly
    } else {
      ::HKEY subKeyHandle = openSubKeyForReading(thisSettingsKeyHandle, categoryName);
      ON_SCOPE_EXIT {
        LSTATUS result = ::RegCloseKey(subKeyHandle);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result == ERROR_SUCCESS) && u8"Registry subkey is successfully closed");
      };

      // Query from subkey
    }
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
