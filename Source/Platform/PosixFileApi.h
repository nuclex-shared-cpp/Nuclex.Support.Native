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

#ifndef NUCLEX_SUPPORT_PLATFORM_POSIXFILEAPI_H
#define NUCLEX_SUPPORT_PLATFORM_POSIXFILEAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include <string> // for std::string
#include <cstdint> // for std::uint8_t
#include <cstdio> // for FILE, ::fopen(), etc.
#include <filesystem> // for std::filesystem

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is a helper class that wraps Posix calls with error checking and conversion
  ///     between C strings and C++ strings so that this boilerplate code does not have
  ///     to be repeated over and over in other places.
  ///   </para>
  /// </remarks>
  class PosixFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>A pointer representing the opened file</returns>
    public: static FILE *OpenFileForReading(const std::filesystem::path &path);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="truncate">Whether the existing file contents are truncated</param>
    /// <returns>A pointer representing the opened file</returns>
    public: static FILE *OpenFileForWriting(const std::filesystem::path &path, bool truncate);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="file">File from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(
      FILE *file, std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Writes data into the specified file</summary>
    /// <param name="file">File into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(
      FILE *file, const std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Flushes all buffered output to the hard drive<summary>
    /// <param name="file">File for whose buffered output will be flushed</param>
    public: static void Flush(FILE *file);

    /// <summary>Closes the specified file</summary>
    /// <param name="file">File pointer returned by the open method</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void Close(FILE *file, bool throwOnError = true);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXFILEAPI_H
