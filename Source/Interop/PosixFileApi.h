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

#ifndef NUCLEX_SUPPORT_INTEROP_POSIXFILEAPI_H
#define NUCLEX_SUPPORT_INTEROP_POSIXFILEAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "./ErrorPolicy.h" // for ErrorPolicy

#include <dirent.h> // struct ::dirent
#include <sys/stat.h> // ::fstat() and permission flags

#include <string> // for std::string
#include <cstdint> // for std::uint8_t
#include <cstdio> // for FILE, ::fopen(), etc.
#include <filesystem> // for std::filesystem

namespace Nuclex::Support::Interop {

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

    /// <summary>Opens a directory for enumeration</summary>
    /// <param name="path">Path of the directory that will be opened</param>
    /// <returns>The opened directory</returns>
    public: static DIR *OpenDirectory(const std::filesystem::path &path);

    /// <summary>Reads the next directory entry from a directory</summary>
    /// <param name="directory">Directory from which the next entry will be read</param>
    /// <returns>
    ///   The next directory entry or NULL if the last directory entry has been reached
    /// </returns>
    public: static struct ::dirent *ReadDirectory(::DIR *directory);

    /// <summary>Closes a directory that was opened for enumeration</summary>
    /// <typeparam name="errorPolicy">
    ///   How to deal with errors that occur when closing the file. Mainly useful for
    ///   RAII situations where the destructor shouldn't throw.
    /// </param>
    /// <param name="directory">Directory that will be closed</param>
    public: template<ErrorPolicy errorPolicy = ErrorPolicy::Throw>
    static void CloseDirectory(::DIR *directory);

    /// <summary>Retrieves the status of the file in the specified path</summary>
    /// <param name="path">Path of the file whose status will be retrieved</param>
    /// <param name="fileStatus">Receives the file status on successfull execution</param>
    /// <returns>True if the file exists and was queried, false if it doesn't exsit</returns>
    /// <remarks>
    ///   If any error other than the file not existing occurs, an exception is thrown.
    /// </remarks>
    public: static bool LStat(
      const std::filesystem::path &path, struct ::stat &fileStatus
    );

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
    /// <typeparam name="errorPolicy">
    ///   How to deal with errors that occur when closing the file. Mainly useful for
    ///   RAII situations where the destructor shouldn't throw.
    /// </param>
    /// <param name="file">File pointer returned by the open method</param>
    public: template<ErrorPolicy errorPolicy = ErrorPolicy::Throw>
    static void Close(FILE *file);

    /// <summary>Deletes a directory and everything in it</summary>
    /// <param name="path">Path of the directory that will be deleted</param>
    /// <returns>True if the file was removed or did not exist in the first place</returns>
    public: static bool RemoveDirectory(const std::filesystem::path &path);

    /// <summary>Deletes a file</summary>
    /// <param name="path">Path of the file that will be deleted</param>
    /// <returns>True if the file was removed or did not exist in the first place</returns>
    public: static bool RemoveFile(const std::filesystem::path &path);

  };

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::Close<ErrorPolicy::Throw>(FILE *file);
  template<> void PosixFileApi::Close<ErrorPolicy::Assert>(FILE *file);

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_INTEROP_POSIXFILEAPI_H
