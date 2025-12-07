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

#ifndef NUCLEX_SUPPORT_INTEROP_WINDOWSREGISTRYAPI_H
#define NUCLEX_SUPPORT_INTEROP_WINDOWSREGISTRYAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

#include <string> // for std::u8string
#include <vector> // for std::vector

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the API used to interface with the registry on Windows systems</summary>
  class WindowsRegistryApi {

    /// <summary>Returns the registry hive matching its string name</summary>
    /// <param name="hiveName">Name of the registry hive whose key will be returned</param>
    /// <returns>The registry hive with the specified hive name</returns>
    /// <remarks>
    ///   This supports both the short form (HKCU/, HKLM/) and the long form
    ///   (HKEY_CURRENT_USER/, HKEY_LOCAL_MACHINE/) for specifying the hive.
    /// </remarks>
    public: static ::HKEY GetHiveFromString(
      const std::u8string &hiveName, std::u8string::size_type hiveNameLength
    );

    /// <summary>
    ///   Builds a list of the names of all registry keys directly below the key with
    ///   the specified handle
    /// </summary>
    /// <param name="keyHandle">Handle of the key whose direct children will be queried</param>
    /// <returns>A list containing the names of all child keys</returns>
    public: static std::vector<std::u8string> GetAllSubKeyNames(
      ::HKEY keyHandle
    );

    /// <summary>
    ///   Builds a list of the names of all value directly below the key with
    ///   the specified handle
    /// </summary>
    /// <param name="keyHandle">Handle of the key whose values will be queried</param>
    /// <returns>A list containing the names of all value below the key</returns>
    public: static std::vector<std::u8string> GetAllValueNames(
      ::HKEY keyHandle
    );

    /// <summary>Opens a subkey below the specified parent registry key</summary>
    /// <param name="parentKeyHandle">Handle of the parent registry key</param>
    /// <param name="subKeyName">Name of the subkey that will be opened</param>
    /// <param name="writable">Whether the key will be opened with write permissions</param>
    /// <returns>
    ///   The handle of the opened registry subkey or a null pointer if the key doesn't exist
    /// </returns>
    public: static ::HKEY OpenExistingSubKey(
      ::HKEY parentKeyHandle, const std::u8string &subKeyName, bool writable = true
    );

    /// <summary>Opens or creates a subkey below the specified parent registry key</summary>
    /// <param name="parentKeyHandle">Handle of the parent registry key</param>
    /// <param name="subKeyName">Name of the subkey that will be opened or created</param>
    /// <returns>The handle of the opened or created registry subkey</returns>
    public: static ::HKEY OpenOrCreateSubKey(
      ::HKEY parentKeyHandle, const std::u8string &subKeyName
    );

    /// <summary>Deletes the specified registry and all subkeys and values in it</summary>
    /// <param name="parentKeyHandle">
    ///   Handle of the key below which the key that will be deleted is located
    /// </param>
    /// <param name="subKeyName">
    ///   Name and or path of a subkey that will be deleted (if a path is specified,
    ///   it's the bottommost key in the path that will be deleted)
    /// </param>
    /// <returns>True if the key existed and was deleted, false if it didn't exist</returns>
    public: static bool DeleteTree(::HKEY parentKeyHandle, const std::u8string &subKeyName);

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_INTEROP_WINDOWSREGISTRYAPI_H
