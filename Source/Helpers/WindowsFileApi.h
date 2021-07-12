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

#ifndef NUCLEX_SUPPORT_HELPERS_WINDOWSFILEAPI_H
#define NUCLEX_SUPPORT_HELPERS_WINDOWSFILEAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

namespace Nuclex { namespace Support { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps file access functions from the Windows file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is just a small helper class that reduces the amount of boilerplate code
  ///     required when calling the file system API functions from Windows, such as
  ///     checking result codes and transforming paths from UTF-8 to UTF-16 stored in
  ///     wchar_ts of non-standard 2 byte size.
  ///   </para>
  ///   <para>
  ///     It is not intended to hide operating system details or make this API platform
  ///     neutral (the File and Container classes do that), so depending on the amount
  ///     of noise required by the file system APIs, only some methods will be wrapped here.
  ///   </para>
  /// </remarks>
  class WindowsFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForReading(const std::string &path);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForWriting(const std::string &path);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="fileHandle">Handle of the file from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(HANDLE fileHandle, void *buffer, std::size_t count);

    /// <summary>Writes data into the specified file</summary>
    /// <param name="fileHandle">Handle of the file into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(HANDLE fileHandle, const void *buffer, std::size_t count);

    /// <summary>Ensures changes to the specified file have been written to disk</summary>
    /// <param name="fileHandle">Handle of the file that will be flushed</param>
    public: static void FlushFileBuffers(HANDLE fileHandle);

    /// <summary>Closes the specified file</summary>
    /// <param name="fileHandle">Handle of the file that will be closed</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void CloseFile(HANDLE fileHandle, bool throwOnError = true);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Helpers

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_HELPERS_WINDOWSFILEAPI_H
