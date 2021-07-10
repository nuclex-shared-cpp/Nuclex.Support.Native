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

#include "PosixFileAccessApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "../../Helpers/PosixApi.h"

#include <cstdio> // for fopen() and fclose()
#include <cerrno> // To access ::errno directly
#include <cassert> // for assert()
#include <limits> // for std::numeric_limits

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileAccessApi::OpenFileForReading(const std::string &path) {
    static const char *fileMode = "rb";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileAccessApi::OpenFileForWriting(const std::string &path) {
    static const char *fileMode = "w+b";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileAccessApi::Read(FILE *file, std::uint8_t *buffer, std::size_t count) {
    size_t readByteCount = ::fread(buffer, 1, count, file);
    if(unlikely(readByteCount == 0)) {
      int errorNumber = errno;

      int result = ::feof(file);
      if(result != 0) {
        return 0; // Read was successful, but end of file has been reached
      }

      std::string errorMessage(u8"Could not read data from file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return static_cast<std::size_t>(readByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileAccessApi::Write(FILE *file, const std::uint8_t *buffer, std::size_t count) {
    size_t writtenByteCount = ::fwrite(buffer, 1, count, file);
    if(unlikely(writtenByteCount == 0)) {
      int errorNumber = errno;

      int result = ::ferror(file);
      if(result == 0) {
        return 0; // Write was successful but no bytes could be written ?_?
      }

      std::string errorMessage(u8"Could not write data to file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return writtenByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileAccessApi::Flush(FILE *file) {
    int result = ::fflush(file);
    if(unlikely(result == EOF)) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not flush file buffers");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileAccessApi::Close(FILE *file, bool throwOnError /* = true */) {
    int result = ::fclose(file);
    if(throwOnError && unlikely(result != 0)) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not close file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Settings::Posix

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
