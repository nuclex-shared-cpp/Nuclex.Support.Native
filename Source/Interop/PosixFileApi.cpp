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
#include <unistd.h> // ::read(), ::write(), ::rmdir(), etc.

#include <cstdio> // for fopen() and fclose()
#include <cerrno> // To access ::errno directly
#include <cassert> // for assert()
#include <limits> // for std::numeric_limits

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  template<> DIR *PosixFileApi::OpenDirectory<ErrorPolicy::Throw>(
    const std::filesystem::path &path
  ) {
    ::DIR *result = ::opendir(path.c_str());
    if(result == nullptr) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open directory '");
      errorMessage.append(path.u8string());
      errorMessage.append(u8"' for enumeration");

      PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> DIR *PosixFileApi::OpenDirectory<ErrorPolicy::Assert>(
    const std::filesystem::path &path
  ) {
    ::DIR *result = ::opendir(path.c_str());
    if(result == nullptr) [[unlikely]] {
      assert((result != nullptr) && u8"Directory is opened for enumeration successfully");
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> struct ::dirent *PosixFileApi::ReadDirectory<ErrorPolicy::Throw>(
    ::DIR *directory
  ) {
    errno = 0;
    struct ::dirent *directoryEntry = ::readdir(directory);
    if(directoryEntry == nullptr) [[unlikely]] {
      int errorNumber = errno;

      // If readdir() returned zero because the last entry is reached, errno stays unchanged
      if(errorNumber == 0) [[likely]] {
        return nullptr;
      } else {
        std::u8string errorMessage(u8"Could not enumerate directory contents");
        PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
      }
    }

    return directoryEntry;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> struct ::dirent *PosixFileApi::ReadDirectory<ErrorPolicy::Assert>(
    ::DIR *directory
  ) {
    errno = 0;
    struct ::dirent *directoryEntry = ::readdir(directory);
    if(directoryEntry == nullptr) [[unlikely]] {
      int errorNumber = errno;

      // If readdir() returned zero because the last entry is reached, errno stays unchanged
      if(errorNumber == 0) [[likely]] {
        return nullptr;
      }

      assert(
        ((directoryEntry != nullptr) || (errorNumber == 0)) &&
        u8"Directory entry is enumerated successfully"
      );
    }

    return directoryEntry;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::CloseDirectory<ErrorPolicy::Throw>(::DIR *directory) {
    int result = ::closedir(directory);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not close directory");
      Interop::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::CloseDirectory<ErrorPolicy::Assert>(::DIR *directory) {
    int result = ::closedir(directory);
    assert((result == 0) && u8"Directory must be closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::LStat<ErrorPolicy::Throw>(
    const std::filesystem::path &path, struct ::stat &fileStatus
  ) {
    int result = ::lstat(path.c_str(), &fileStatus);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      // This is an okay outcome for us: the file or directory does not exist.
      if((errorNumber == ENOENT) || (errorNumber == ENOTDIR)) {
        return false;
      }

      std::u8string errorMessage(u8"Could not obtain file status for '");
      errorMessage.append(path.u8string());
      errorMessage.push_back(u8'\'');

      PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::LStat<ErrorPolicy::Assert>(
    const std::filesystem::path &path, struct ::stat &fileStatus
  ) {
    int result = ::lstat(path.c_str(), &fileStatus);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      // This is an okay outcome for us: the file or directory does not exist.
      if((errorNumber == ENOENT) || (errorNumber == ENOTDIR)) {
        return false;
      }

      assert(
        ((result == 0) || (errorNumber == ENOENT) || (errorNumber == ENOTDIR)) &&
        u8"File status is queried successfully"
      );
      return false;
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForReading(const std::filesystem::path &path) {
    static const char *fileMode = "rb";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(file == nullptr) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open file '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, path);
      errorMessage.append(u8"' for reading");

      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
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

      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
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
      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
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
      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return writtenByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Flush(FILE *file) {
    int result = ::fflush(file);
    if(result == EOF) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not flush file buffers");
      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::Close<ErrorPolicy::Throw>(FILE *file) {
    int result = ::fclose(file);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not close file");
      Interop::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void PosixFileApi::Close<ErrorPolicy::Assert>(FILE *file) {
    int result = ::fclose(file);
    assert((result == 0) && u8"File must be closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::RemoveDirectory<ErrorPolicy::Throw>(
    const std::filesystem::path &path
  ) {
    int result = ::rmdir(path.c_str());
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      if(errorNumber == ENOENT) [[unlikely]] {
        return true;
      }

      std::u8string errorMessage(u8"Could not remove directory '");
      errorMessage.append(path.u8string());
      errorMessage.push_back(u8'\'');

      PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::RemoveDirectory<ErrorPolicy::Assert>(
    const std::filesystem::path &path
  ) {
    int result = ::rmdir(path.c_str());
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      if(errorNumber == ENOENT) [[unlikely]] {
        return true;
      }

      assert(
        ((result == 0) || (errorNumber != ENOENT)) &&
        u8"Directory must be deleted successfully"
      );
      return false;
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::RemoveFile<ErrorPolicy::Throw>(
    const std::filesystem::path &path
  ) {
    int result = ::unlink(path.c_str());
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      if(errorNumber == ENOENT) [[unlikely]] {
        return true; // The desired outcome is achieved: the file doesn't exist :)
      }

      std::u8string errorMessage(u8"Could not delete file '");
      errorMessage.append(path.u8string());
      errorMessage.push_back(u8'\'');

      PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool PosixFileApi::RemoveFile<ErrorPolicy::Assert>(
    const std::filesystem::path &path
  ) {
    int result = ::unlink(path.c_str());
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;

      if(errorNumber == ENOENT) [[unlikely]] {
        return true; // The desired outcome is achieved: the file doesn't exist :)
      }

      assert(
        ((result == 0) || (errorNumber != ENOENT)) &&
        u8"File must be deleted successfully"
      );
      return false;
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
