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

#include "LinuxFileApi.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "Nuclex/Support/Text/StringConverter.h" // for StringConverter
#include "PosixApi.h" // Linux uses Posix error handling
#include "PosixPathApi.h" // Path manipulation stuff for ::mk*temp()

#include <linux/limits.h> // for PATH_MAX
#include <fcntl.h> // ::open() and flags
#include <unistd.h> // ::read(), ::write(), ::close(), etc.

#include <cassert> // assert()
#include <cerrno> // To access ::errno directly
#include <vector> // std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //
#if 0 // If temporary file/directory methods are moved to this file
  /// <summary>Builds the template string that's passed to ::mkstemp()/::mkdtemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMkTemp(std::u8string &path, const std::u8string &prefix) {
    path.reserve(256); // PATH_MAX would be a bit too bloaty usually...

    // Obtain the system's temporary directory (usually /tmp, can be overridden)
    //   path: "/tmp/"
    {
      Nuclex::Support::Platform::PosixPathApi::GetTemporaryDirectory(path);

      std::u8string::size_type length = path.size();
      if(path[length - 1] != u8'/') {
        path.push_back(u8'/');
      }
    }

    // Append the user-specified prefix, if any
    //   path: "/tmp/myapp"
    if(!prefix.empty()) {
      path.append(prefix);
    }

    // Append the mandatory placeholder characters
    //   path: "/tmp/myappXXXXXX"
    {
      static const std::u8string placeholder(u8"XXXXXX", 6);
      path.append(placeholder);
    }
  }
#endif // 0
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileApi::OpenFileForReading(const std::filesystem::path &path) {
    int fileDescriptor = ::open(path.c_str(), O_RDONLY | O_LARGEFILE);
    if(fileDescriptor < 0) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open file '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, path);
      errorMessage.append(u8"' for reading");

      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileApi::OpenFileForWriting(const std::filesystem::path &path) {
    int fileDescriptor = ::open(
      path.c_str(),
      O_RDWR | O_CREAT | O_LARGEFILE,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
    );
    if(fileDescriptor < 0) [[unlikely]] {
      int errorNumber = errno;

      std::u8string errorMessage(u8"Could not open file '");
      Text::StringConverter::AppendPathAsUtf8(errorMessage, path);
      errorMessage.append(u8"' for writing");

      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Seek(int fileDescriptor, ::off_t offset, int anchor) {
    ::off_t absolutePosition = ::lseek(fileDescriptor, offset, anchor);
    if(absolutePosition == -1) {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not seek within file");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return static_cast<std::size_t>(absolutePosition);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Read(
    int fileDescriptor, std::byte *buffer, std::size_t count
  ) {
    ssize_t result = ::read(fileDescriptor, buffer, count);
    if(result == static_cast<ssize_t>(-1)) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not read data from file");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Write(
    int fileDescriptor, const std::byte *buffer, std::size_t count
  ) {
    ssize_t result = ::write(fileDescriptor, buffer, count);
    if(result == static_cast<ssize_t>(-1)) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not write data to file");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFileApi::SetLength(int fileDescriptor, std::size_t byteCount) {
    int result = ::ftruncate(fileDescriptor, static_cast<::off_t>(byteCount));
    if(result == -1) {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not truncate/pad file to specified length");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFileApi::Flush(int fileDescriptor) {
    int result = ::fsync(fileDescriptor);
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not flush file buffers");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void LinuxFileApi::Close<ErrorPolicy::Throw>(int fileDescriptor) {
    int result = ::close(fileDescriptor);
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      std::u8string errorMessage(u8"Could not close file");
      Platform::PosixApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void LinuxFileApi::Close<ErrorPolicy::Assert>(int fileDescriptor) {
    int result = ::close(fileDescriptor);
    assert((result != -1) && u8"File must be closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)
