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

#ifndef NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H
#define NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "PosixApi.h"

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps or reimplements the Posix path API</summary>
  class PosixPathApi {

    /// <summary>Checks if the specified path is a relative path</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path is a relative path</returns>
    public: static bool IsPathRelative(const std::string &path);

    /// <summary>Appends one path to another</summary>
    /// <param name="path">Path to which another path will be appended</param>
    /// <param name="extra">Other path that will be appended</param>
    public: static void AppendPath(std::string &path, const std::string &extra);

    /// <summary>Removes the file name from a path containing a file name</summary>
    /// <param name="path">Path from which the file name will be removed</param>
    public: static void RemoveFileFromPath(std::string &path);

    /// <summary>Checks if the specified path exists and if it is a file</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path exists and is a file, false otherwise</returns>
    public: static bool DoesFileExist(const std::string &path);

    /// <summary>Determines the path of the user's temporary directory</summary>
    /// <param name="target">
    ///   String in which the full path of the temporary directory will be placed
    /// </param>
    public: static void GetTemporaryDirectory(std::string &path);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H
