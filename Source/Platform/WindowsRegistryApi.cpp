#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "WindowsRegistryApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/StringConverter.h" // for StringConverter
#include "Nuclex/Support/Text/UnicodeHelper.h" // for UnicodeHelper

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether a string starts with a specified sequence of characters</summary>
  /// <param name="text">Text whose starting characters will be checked</param>
  /// <param name="beginningUppercase">
  ///   Character sequence the string's beginning is checked against in uppercase
  /// </param>
  /// <param name="beginningLowercase">
  ///   Character sequence the string's beginning is checked against in lowercase
  /// </param>
  /// <param name="length">Length of the character sequence in bytes</param>
  /// <returns>
  ///   True if the string specified via <paramref name="text" /> starts with
  ///   the provided character sequence either in uppercase, lowercase or any mix
  ///   of both. False otherwise.
  /// </returns>
  bool startsWith(
    const std::string_view &text,
    const char *beginningUppercase,
    const char *beginningLowercase,
    std::string_view::size_type length
  ) {
    assert((text.length() >= length) && u8"Text is long enough to compare");

    for(std::string_view::size_type index = 0; index < length; ++index) {
      char current = text[index];
      bool match = (
        (current == beginningUppercase[index]) ||
        (current == beginningLowercase[index])
      );
      if(!match) {
        return false;
      }
    }

    return true;
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

  /// <summary>Changes all slashes in a UTF-8 string to backward slashes</summary>
  /// <param name="stringToChange">String in which the slashes will be changed</param>
  void makeAllSlashesBackward(std::wstring &stringToChange) {
    std::string::size_type length = stringToChange.length();
    for(std::string::size_type index = 0; index < length; ++index) {
      if(stringToChange[index] == L'/') {
        stringToChange[index] = L'\\';
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring wideFromUtf8AndUseBackwardSlashes(const std::string &utf8String) {
    using Nuclex::Support::Text::UnicodeHelper;
    typedef UnicodeHelper::Char8Type Char8Type;

    std::wstring result;
    {
      const Char8Type *read = reinterpret_cast<const Char8Type *>(utf8String.c_str());
      const Char8Type *readEnd = read + utf8String.length();

      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      // Variant for 16 bit wchar_t as established by Windows compilers
      if constexpr(sizeof(wchar_t) == sizeof(char16_t)) {
        char16_t *write = reinterpret_cast<char16_t *>(result.data());

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          if(unlikely(codePoint == U'/')) {
            UnicodeHelper::WriteCodePoint(write, U'\\');
          } else {
            UnicodeHelper::WriteCodePoint(write, codePoint);
          }
        }

        result.resize(write - reinterpret_cast<char16_t *>(result.data()));
      } else { // Variant for 32 bit wchar_t used everywhere except Windows
        char32_t *write = reinterpret_cast<char32_t *>(result.data());

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          if(unlikely(codePoint == U'/')) {
            *write = U'\\';
          } else {
            *write = UnicodeHelper::ReadCodePoint(read, readEnd);
          }
          ++write;
        }

        result.resize(write - reinterpret_cast<char32_t *>(result.data()));
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  ::HKEY WindowsRegistryApi::GetHiveFromString(
    const std::string &hiveName, std::string::size_type hiveNameLength
  ) {
    if(hiveNameLength >= 3) {
      bool isHk = (
        ((hiveName[0] == 'H') || (hiveName[0] == 'h')) &&
        ((hiveName[1] == 'K') || (hiveName[1] == 'k'))
      );
      if(isHk) {

        // Is the prefix 'HKU' for HKEY_USERS?
        if(hiveNameLength == 3) {
          if((hiveName[2] == 'U') || (hiveName[2] == 'u')) {
            return HKEY_USERS;
          }
        }

        if(hiveNameLength == 4) {

          // Is the prefix 'HKCR', 'HKCU' or 'HKCC'?
          if((hiveName[2] == 'C') || (hiveName[2] == 'c')) {
            if((hiveName[3] == 'R') || (hiveName[3] == 'r')) {
              return HKEY_CLASSES_ROOT;
            }
            if((hiveName[3] == 'U') || (hiveName[3] == 'u')) {
              return HKEY_CURRENT_USER;
            }
            if((hiveName[3] == 'C') || (hiveName[3] == 'c')) {
              return HKEY_CURRENT_CONFIG;
            }
          }

          // Is the prefix 'HKLM' for HKEY_LOCAL_MACHINE?
          bool isHklm = (
            ((hiveName[2] == 'L') || (hiveName[2] == 'l')) &&
            ((hiveName[3] == 'M') || (hiveName[3] == 'm'))
          );
          if(isHklm) {
            return HKEY_LOCAL_MACHINE;
          }

        } // hiveNameLength == 4
      } // isHk
    } // hiveNameLength >= 3

    // Check for the full names of the registry hives. We do a naive byte-by-byte
    // comparison via startsWith(), but that's totally fine. If the hiveName string
    // contains unicode characters, they're guaranteed mismatches, the only matching
    // byte sequences are the ones actually producing the compared registry hive names.

    if(hiveNameLength == 10) {
      if(startsWith(hiveName, u8"HKEY_USERS", u8"hkey_users", 10)) {
        return HKEY_USERS;
      }
    }
    if(hiveNameLength == 17) {
      if(startsWith(hiveName, u8"HKEY_CLASSES_ROOT", u8"hkey_classes_root", 17)) {
        return HKEY_CLASSES_ROOT;
      }
      if(startsWith(hiveName, u8"HKEY_CURRENT_USER", u8"hkey_current_user", 17)) {
        return HKEY_CURRENT_USER;
      }
    }
    if(hiveNameLength == 18) {
      if(startsWith(hiveName, u8"HKEY_LOCAL_MACHINE", u8"hkey_local_machine", 18)) {
        return HKEY_LOCAL_MACHINE;
      }
    }
    if(hiveNameLength == 19) {
      if(startsWith(hiveName, u8"HKEY_CURRENT_CONFIG", u8"hkey_current_config", 19)) {
        return HKEY_CURRENT_CONFIG;
      }
    }

    // No match found, return a null pointer to let the caller know
    return nullptr;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> WindowsRegistryApi::GetAllSubKeyNames(::HKEY keyHandle) {
    DWORD subKeyCount, longestSubKeyLength;
    {
      ::LSTATUS result = ::RegQueryInfoKeyW(
        keyHandle,
        nullptr, nullptr, nullptr, &subKeyCount,
        &longestSubKeyLength, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
      );
      if(unlikely(result != ERROR_SUCCESS)) {
        Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query number of subkeys from registry key", result
        );
      }
    }

    // Collect a list of all subkeys below the root settings key
    std::vector<std::string> results;
    if(subKeyCount > 0) {
      results.reserve(subKeyCount);

      std::wstring keyName(longestSubKeyLength, std::wstring::value_type(0));
      DWORD keyNameLength = longestSubKeyLength;

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
            keyHandle,
            index,
            keyName.data(),
            &keyNameLength,
            nullptr, nullptr, nullptr, nullptr
          );
          if(likely(result != ERROR_MORE_DATA)) {
            break;
          } else {
            longestSubKeyLength += 256;
            keyName.resize(longestSubKeyLength);
            keyNameLength = longestSubKeyLength;
          }
        }
        if(unlikely(result == ERROR_NO_MORE_ITEMS)) {
          break; // end reached
        } else if(unlikely(result != ERROR_SUCCESS)) {
          Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not query name of subkey from registry key", result
          );
        }

        // The registry API is, like any Windows API, bogged down with Microsoft's
        // poor choice of using UTF-16. So we need to convert everything returned by
        // said method into UTF-8 ourselves.
        keyName.resize(keyNameLength);
        results.push_back(Text::StringConverter::Utf8FromWide(keyName));
        keyName.resize(longestSubKeyLength);

      } // for each enumerated registry subkey
    } // if at least one subkey present

    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> WindowsRegistryApi::GetAllValueNames(::HKEY keyHandle) {
    // Query the number of subkeys in our root settings key
    DWORD valueCount;
    DWORD longestValueNameLength;
    {
      ::LSTATUS result = ::RegQueryInfoKeyW(
        keyHandle,
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, &valueCount, &longestValueNameLength,
        nullptr, nullptr, nullptr
      );
      if(unlikely(result != ERROR_SUCCESS)) {
        Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query number of values in registry key", result
        );
      }
    }

    // Collect a list of all subkeys below the root settings key
    std::vector<std::string> results;
    if(valueCount > 0) {
      results.reserve(valueCount);

      std::wstring valueName(longestValueNameLength, std::wstring::value_type(0));
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
            keyHandle,
            index,
            valueName.data(),
            &valueNameLength,
            nullptr, nullptr, nullptr, nullptr
          );
          if(likely(result != ERROR_MORE_DATA)) {
            break;
          } else {
            longestValueNameLength += 256;
            valueName.resize(longestValueNameLength);
            valueNameLength = longestValueNameLength;
          }
        }
        if(unlikely(result == ERROR_NO_MORE_ITEMS)) {
          break; // end reached
        } else if(unlikely(result != ERROR_SUCCESS)) {
          Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not query name of registry value", result
          );
        }

        // The registry API is, like any Windows API, bogged down with Microsoft's
        // poor choice of using UTF-16. So we need to convert everything returned by
        // said method into UTF-8 ourselves.
        valueName.resize(valueNameLength);
        results.push_back(Text::StringConverter::Utf8FromWide(valueName));
        valueName.resize(longestValueNameLength);

      } // for each enumerated registry value
    }  // if at least one value present

    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  ::HKEY WindowsRegistryApi::OpenExistingSubKey(
    ::HKEY parentKeyHandle, const std::string &subKeyName, bool writable /* = true */
  ) {
    ::HKEY subKeyHandle;
    {
      // Flags to tell the "security accounts manager" what access level we need
      ::REGSAM desiredAccessLevel = KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS;
      if(writable) {
        desiredAccessLevel |= (KEY_SET_VALUE | KEY_CREATE_SUB_KEY);
      }

      // Attempt to open the key. This does not create a key if it doesn't exist
      ::LSTATUS result;
      if(subKeyName.empty()) {
        result = ::RegOpenKeyExW(
          parentKeyHandle,
          nullptr, // subkey name
          0, // options
          desiredAccessLevel,
          &subKeyHandle
        );
      } else {
        std::wstring subKeyNameUtf16 = Text::StringConverter::WideFromUtf8(subKeyName);
        result = ::RegOpenKeyExW(
          parentKeyHandle,
          subKeyNameUtf16.c_str(),
          0, // options
          desiredAccessLevel,
          &subKeyHandle
        );
      }
      if(unlikely(result != ERROR_SUCCESS)) {
        if(result == ERROR_FILE_NOT_FOUND) {
          return ::HKEY(nullptr);
        } else {
          Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not open registry subkey", result
          );
        }
      }
    }

    return subKeyHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  ::HKEY WindowsRegistryApi::OpenOrCreateSubKey(
    ::HKEY parentKeyHandle, const std::string &subKeyName
  ) {
    ::HKEY openedSubKey;
    {
      std::wstring subKeyNameUtf16 = Text::StringConverter::WideFromUtf8(subKeyName);
      ::LSTATUS result = ::RegCreateKeyExW(
        parentKeyHandle,
        subKeyNameUtf16.c_str(),
        0, // reserved
        nullptr, // class ("user-defined type of this key" - no clue)
        REG_OPTION_NON_VOLATILE,
        KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
        nullptr, // security attributes
        &openedSubKey,
        nullptr // disposition - tells whether new key was created - we don't care
      );
      if(unlikely(result != ERROR_SUCCESS)) {
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not open or create registry subkey for read/write access", result
        );
      }
    }

    return openedSubKey;
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsRegistryApi::DeleteTree(::HKEY parentKeyHandle, const std::string &subKeyName) {
    std::wstring subKeyNameUtf16 = Text::StringConverter::WideFromUtf8(subKeyName);

    ::LSTATUS result = ::RegDeleteTreeW(parentKeyHandle, subKeyNameUtf16.c_str());
    if(result == ERROR_FILE_NOT_FOUND) {
      return false;
    } else if(unlikely(result != ERROR_SUCCESS)) {
      std::string message(u8"Could not delete registry tree at '", 35);
      message.append(subKeyName);
      message.append(u8"'", 1);
      Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
