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

#include "Nuclex/Support/TemporaryFileScope.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)
#include "Platform/LinuxFileApi.h"
#include "Platform/PosixApi.h"

#include <unistd.h> // for ::write(), ::close(), ::unlink()
#include <cstdlib> // for ::getenv(), ::mkdtemp()
#else
#include "Platform/WindowsApi.h" // for WindowsApi
#include "Platform/WindowsFileApi.h" // for WindowsApi
#endif

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Appends the user's/system's preferred temp directory to a path</summary>
  /// <param name="path">Path vector the temp directory will be appended to</param>
  void appendTempDirectory(std::vector<char> &path) {

    // Obtain the most likely system temp directory
    const char *tempDirectory = ::getenv(u8"TMPDIR");
    if(tempDirectory == nullptr) {
      tempDirectory = ::getenv(u8"TMP");
      if(tempDirectory == nullptr) {
        tempDirectory = ::getenv(u8"TEMP");
        if(tempDirectory == nullptr) {
          // This is safe (part of the file system standard and Linux standard base),
          // but we wanted to honor any possible user preferences first.
          tempDirectory = u8"/tmp";
        }
      }
    }

    // Append the temporary directory to the path vector
    while(*tempDirectory != 0) {
      path.push_back(*tempDirectory);
      ++tempDirectory;
    }

  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Builds the full template string that's passed to mkostemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMkosTemp(std::vector<char> &path, const std::string &prefix) {
      path.reserve(256); // PATH_MAX would be a bit too bloaty usually...

    // Obtain the system's temporary directory (usually /tmp, can be overridden)
    //   path: "/tmp/"
    {
      appendTempDirectory(path);
      {
        std::string::size_type length = path.size();
        if(path[length -1] != '/') {
          path.push_back('/');
        }
      }
    }

    // Append the user-specified prefix, if any
    //   path: "/tmp/myapp"
    if(!prefix.empty()) {
      path.insert(path.end(), prefix.begin(), prefix.end());
    }

    // Append the mandatory placeholder characters
    //   path: "/tmp/myappXXXXXX"
    {
      static const std::string placeholder(u8"XXXXXX", 6);
      path.insert(path.end(), placeholder.begin(), placeholder.end());
    }
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::TemporaryFileScope(const std::string &namePrefix /* = u8"tmp" */) :
    path(),
    privateImplementationData {0} {
#if !defined(NUCLEX_SUPPORT_WINDOWS)
    static_assert(
      (sizeof(this->privateImplementationData) >= sizeof(int)) &&
      u8"File descriptor fits in space provided for private implementation data"
    );

    std::vector<char> pathTemplate;
    buildTemplateForMkosTemp(pathTemplate, namePrefix);

    // Select and open a unique temporary filename
    int fileDescriptor = ::mkstemp(pathTemplate.data());
    if(unlikely(fileDescriptor == -1)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not create temporary file '");
      errorMessage.append(pathTemplate.data());
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    // Store the file handle in the private implementation data block and
    // remember the full path for when the user queries it later
    *reinterpret_cast<int *>(this->privateImplementationData) = fileDescriptor;
    this->path.assign(pathTemplate.begin(), pathTemplate.end());
#else
    throw u8"Not implemented yet";
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::~TemporaryFileScope() {
#if !defined(NUCLEX_SUPPORT_WINDOWS)
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);

    if(likely(fileDescriptor != 0)) {
      int result = ::close(fileDescriptor);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is closed successfully");
    }

    if(likely(!this->path.empty())) {
      int result = ::unlink(this->path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is deleted successfully");
    }
#else
    throw u8"Not implemented yet";
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryFileScope::SetFileContents(
    const std::uint8_t *contents, std::size_t byteCount
  ) {
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);
    assert((fileDescriptor != 0) && u8"File is opened and accessible");

    Platform::LinuxFileApi::Seek(fileDescriptor, ::off_t(0), SEEK_SET);
    Platform::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
    Platform::LinuxFileApi::SetLength(fileDescriptor, byteCount);
    Platform::LinuxFileApi::Flush(fileDescriptor);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
