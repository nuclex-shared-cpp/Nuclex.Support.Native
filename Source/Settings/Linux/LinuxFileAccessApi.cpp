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

#include "LinuxFileAccessApi.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "../../Helpers/PosixApi.h" // Linux uses Posix error handling

#include <linux/limits.h> // for PATH_MAX
#include <fcntl.h> // ::open() and flags
#include <unistd.h> // ::read(), ::write(), ::rmdir(), etc.

#include <cerrno> // To access ::errno directly
#include <vector> // std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings { namespace Linux {

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileAccessApi::OpenFileForReading(const std::string &path) {
    int fileDescriptor = ::open(path.c_str(), O_RDONLY | O_LARGEFILE);
    if(unlikely(fileDescriptor < 0)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileAccessApi::OpenFileForWriting(const std::string &path) {
    int fileDescriptor = ::open(
      path.c_str(),
      O_RDWR | O_CREAT | O_LARGEFILE,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
    );
    if(unlikely(fileDescriptor < 0)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileAccessApi::Read(
    int fileDescriptor, std::uint8_t *buffer, std::size_t count
  ) {
    ssize_t result = ::read(fileDescriptor, buffer, count);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not read data from file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileAccessApi::Write(
    int fileDescriptor, const std::uint8_t *buffer, std::size_t count
  ) {
    ssize_t result = ::write(fileDescriptor, buffer, count);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not write data to file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFileAccessApi::Flush(int fileDescriptor) {
    int result = ::fsync(fileDescriptor);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not flush file buffers");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFileAccessApi::Close(int fileDescriptor, bool throwOnError /* = true */) {
    int result = ::close(fileDescriptor);
    if(throwOnError && unlikely(result == -1)) {
      int errorNumber = errno;
      std::string errorMessage(u8"Could not close file");
      Helpers::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Settings::Linux

#endif // defined(NUCLEX_SUPPORT_LINUX)