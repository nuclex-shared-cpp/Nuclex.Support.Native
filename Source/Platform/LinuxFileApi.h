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

#ifndef NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H
#define NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "./ErrorPolicy.h" // for ErrorPolicy

#include <string> // std::u8string
#include <cstdint> // std::uint8_t and std::size_t
#include <filesystem> // for std::filesystem

#include <sys/stat.h> // ::fstat() and permission flags
#include <dirent.h> // struct ::dirent

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Linux file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is just a small helper class that reduces the amount of boilerplate code
  ///     required when calling the file system API functions, such as checking
  ///     result codes over and over again.
  ///   </para>
  ///   <para>
  ///     It is not intended to hide operating system details or make this API platform
  ///     neutral (the File and Container classes do that), so depending on the amount
  ///     of noise required by the file system APIs, only some methods will be wrapped here.
  ///   </para>
  /// </remarks>
  class LinuxFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The descriptor (numeric handle) of the opened file</returns>
    public: static int OpenFileForReading(const std::filesystem::path &path);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The descriptor (numeric handle) of the opened file</returns>
    public: static int OpenFileForWriting(const std::filesystem::path &path);

    /// <summary>Changes the position of the file cursor</summary>
    /// <param name="fileDescriptor">File handle whose file cursor will be moved</param>
    /// <param name="offset">Relative position, in bytes, to move the file cursor to</param>
    /// <param name="anchor">Position to which the offset is relative</param>
    /// <returns>The absolute position in bytes from the beginning of the file</returns>
    public: static std::size_t Seek(int fileDescriptor, ::off_t offset, int anchor);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="fileDescriptor">Handle of the file from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(
      int fileDescriptor, std::byte *buffer, std::size_t count
    );

    /// <summary>Writes data into the specified file</summary>
    /// <param name="fileDescriptor">Handle of the file into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(
      int fileDescriptor, const std::byte *buffer, std::size_t count
    );

    /// <summary>Truncates or pads the file to the specified length</summary>
    /// <param name="fileDescriptor">Handle of the file whose length will be set</param>
    /// <param name="byteCount">New length fo the file in bytes</param>
    public: static void SetLength(int fileDescriptor, std::size_t byteCount);

    /// <summary>Flushes all buffered output to the hard drive<summary>
    /// <param name="fileDescriptor">
    ///   File descriptor whose buffered output will be flushed
    /// </param>
    public: static void Flush(int fileDescriptor);

    /// <summary>Closes the specified file</summary>
    /// <typeparam name="errorPolicy">
    ///   How to deal with errors that occur when closing the file. Mainly useful for
    ///   RAII situations where the destructor shouldn't throw.
    /// </param>
    /// <param name="fileDescriptor">Handle of the file that will be closed</param>
    public: template<ErrorPolicy errorPolicy = ErrorPolicy::Throw>
    static void Close(int fileDescriptor);

  };

  // ------------------------------------------------------------------------------------------- //

  template<> void LinuxFileApi::Close<ErrorPolicy::Throw>(int fileDescriptor);
  template<> void LinuxFileApi::Close<ErrorPolicy::Assert>(int fileDescriptor);

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H
