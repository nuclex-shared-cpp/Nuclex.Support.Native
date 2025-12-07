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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "PosixFileApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/StringConverter.h" // for StringConverter
#include "PosixApi.h"

#include <cstdio> // for fopen() and fclose()
#include <cerrno> // To access ::errno directly
#include <cassert> // for assert()
#include <limits> // for std::numeric_limits

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForReading(const std::filesystem::path &path) {
    static const char *fileMode = "rb";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(file == nullptr) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open file '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, path);
      errorMessage.append(u8"' for reading");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForWriting(const std::filesystem::path &path, bool truncate) {
    static const char *fileMode = truncate ? "wb" : "w+b";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(file == nullptr) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open file '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, path);
      errorMessage.append(u8"' for writing");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Read(FILE *file, std::uint8_t *buffer, std::size_t count) {
    size_t readByteCount = ::fread(buffer, 1, count, file);
    if(readByteCount == 0) [[unlikely]] {
      int errorNumber = errno;

      int result = ::feof(file);
      if(result != 0) {
        return 0; // Read was successful, but end of file has been reached
      }

      std::u8string errorMessage(u8"Could not read data from file");
      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return static_cast<std::size_t>(readByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Write(FILE *file, const std::uint8_t *buffer, std::size_t count) {
    size_t writtenByteCount = ::fwrite(buffer, 1, count, file);
    if(writtenByteCount == 0) [[unlikely]] {
      int errorNumber = errno;

      int result = ::ferror(file);
      if(result == 0) {
        return 0; // Write was successful but no bytes could be written ?_?
      }

      std::u8string errorMessage(u8"Could not write data to file");
      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return writtenByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Flush(FILE *file) {
    int result = ::fflush(file);
    if(result == EOF) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not flush file buffers");
      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::Close<ErrorPolicy::Throw>(FILE *file) {
    int result = ::fclose(file);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not close file");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::Close<ErrorPolicy::Assert>(FILE *file) {
    int result = ::fclose(file);
    assert((result == 0) && u8"File must be closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
