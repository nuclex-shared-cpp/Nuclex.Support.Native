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

#include "WindowsRegistryApi.h"

#if defined(NUCLEX_SUPPORT_WIN32)

//#include "../../Text/Utf8Fold/Utf8Fold.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  bool startsWith(
    const std::string_view &text,
    const char *beginningUppercase,
    const char *beginningLowercase,
    std::string_view::size_type length
  ) {
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

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  ::HKEY WindowsRegistryApi::GetHiveFromString(const std::string_view &hiveName) {
    std::string_view::size_type hiveNameLength = hiveName.length();

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

}}}} // namespace Nuclex::Support::Settings::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)
