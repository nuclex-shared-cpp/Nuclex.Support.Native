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

#ifndef NUCLEX_SUPPORT_PLATFORM_WINDOWSPATHAPI_H
#define NUCLEX_SUPPORT_PLATFORM_WINDOWSPATHAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Windows path API</summary>
  class WindowsPathApi {

    /// <summary>Checks if the specified path is a relative path</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path is a relative path</returns>
    public: static bool IsPathRelative(const std::wstring &path);

    /// <summary>Appends one path to another</summary>
    /// <param name="path">Path to which another path will be appended</param>
    /// <param name="extra">Other path that will be appended</param>
    public: static void AppendPath(std::wstring &path, const std::wstring &extra);

    /// <summary>Removes the file name from a path containing a file name</summary>
    /// <param name="path">Path from which the file name will be removed</param>
    public: static void RemoveFileFromPath(std::wstring &path);

    /// <summary>Checks whether the specified path has a filename extension</summary>
    /// <param name="path">Path that will be checked for having an extension</param>
    /// <returns>True if the path has a filename extension, false otherwise</returns>
    public: static bool HasExtension(const std::wstring &path);

    /// <summary>Checks if the specified path exists and if it is a file</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path exists and is a file, false otherwise</returns>
    public: static bool DoesFileExist(const std::wstring &path);

    /// <summary>Discovers the Windows system directory</summary>
    /// <param name="target">
    ///   String in which the full path to the Windows system directory will be placed
    /// </param>
    public: static void GetSystemDirectory(std::wstring &target);

    /// <summary>Discovers the Windows directory</summary>
    /// <param name="target">
    ///   String in which the full path to the Windows directory will be placed
    /// </param>
    public: static void GetWindowsDirectory(std::wstring &target);

    /// <summary>Determines the path of the user's temporary directory</summary>
    /// <param name="target">
    ///   String in which the full path of the temporary directory will be placed
    /// </param>
    public: static void GetTemporaryDirectory(std::wstring &target);

    /// <summary>Creates a temporary file with a unique name on Windows systems</summary>
    /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
    /// <returns>The full path to the newly created temporary file</returns>
    public: static std::wstring CreateTemporaryFile(const std::string &prefix);

    /// <summary>Creates a new directory in the specified location</summary>
    /// <param name="path">Path in which the new directory will be created</param>
    public: static void CreateDirectory(const std::wstring &path);

#if defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
    /// <summary>Removes the filename from a full path</summary>
    /// <param name="pszPath">Path from which the filename will be removed</param>
    /// <returns>TRUE if the filename was removed, FALSE if nothing was removed</returns>
    /// <remarks>
    ///   This is a reimplementation of the same-named method from Microsoft's shlwapi,
    ///   done so we don't have to link shlwapi for three measly methods.
    /// </remarks>
    public: static BOOL PathRemoveFileSpecW(LPWSTR pszPath);

    /// <summary>Checks whether a path is relative</summary>
    /// <param name="pszPath">Path that will be checked for being relative</param>
    /// <returns>TRUE if the path is relative, FALSE if it is absolute</returns>
    /// <remarks>
    ///   This is a reimplementation of the same-named method from Microsoft's shlwapi,
    ///   done so we don't have to link shlwapi for three measly methods.
    /// </remarks>
    public: static BOOL PathIsRelativeW(LPCWSTR pszPath);

    /// <summary>Appends a directory or filename to an existing path</summary>
    /// <param name="pszPath">Path to which the other path will be appended</param>
    /// <param name="pszMore">Other path that will be appended to the first path</param>
    /// <returns>TRUE if the path is relative, FALSE if it is absolute</returns>
    /// <remarks>
    ///   This is a reimplementation of the same-named method from Microsoft's shlwapi,
    ///   done so we don't have to link shlwapi for three measly methods.
    ///   Unlike the original, this can destroy pszPath if it starts with dots ('.')
    ///   and the combined path does not fit in the buffer. This library does not
    ///   encounter that case, but if you want to rip this code, you should be aware :)
    /// </remarks>
    BOOL PathAppendW(LPWSTR pszPath, LPCWSTR pszMore);
#endif // defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_WINDOWSPATHAPI_H
