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

#include "PosixProcessApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "PosixPathApi.h"

//#include "Nuclex/Support/Text/LexicalAppend.h"

#include <cstdlib> // for ::getenv(), ::ldiv(), ::ldiv_t
#include <unistd.h> // for ::pipe(), ::readlink()
#include <fcntl.h> // for ::fcntl()
#include <signal.h> // for ::kill()
#include <limits.h> // for PATH_MAX

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the path of the process image file for the runnign application
  /// </summary>
  /// <param name="target">String that will receive the path of the executable</param>
  void getExecutablePath(std::filesystem::path &target) {
    const static std::u8string procSelfExe(u8"/proc/self/exe", 14);
    const static std::u8string slashExe(u8"/exe", 4);

    std::string buffer(PATH_MAX, ' ');

    // Try to read the symlink to obtain the path to the running executable
    std::string ownProcessLink(procSelfExe.begin(), procSelfExe.end());
    ::ssize_t characterCount = ::readlink(ownProcessLink.c_str(), buffer.data(), PATH_MAX);
    if(characterCount == -1) {
      int errorNumber = errno;
      if((errorNumber != EACCES) && (errorNumber != ENOTDIR) && (errorNumber != ENOENT)) {
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not follow '/proc/self/exe' to own path", errorNumber
        );
      }

      // Try it using the numeric process id in case /proc/self/exe isn't found
      {
        ownProcessLink.resize(6); // leaves '/proc/' behind
        ownProcessLink.append(std::to_string(::getpid()));
        ownProcessLink.append(slashExe.begin(), slashExe.end());
      }
      if(characterCount == -1) {
        // Let's stay with the original error message, /proc/self/exe gives
        // the user a much better idea at what the application wanted to do than
        // a random PID that doesn't exist anymore after the error is printed.
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not follow '/proc/self/exe' to own path", errorNumber
        );
      }
    }

    buffer.resize(characterCount);
    target = std::filesystem::path(buffer).remove_filename();
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  Pipe::Pipe() :
    ends {-1, -1} {

    int result = ::pipe(this->ends);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not set up a pipe", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Pipe::~Pipe() {
    if(this->ends[1] != -1) {
      int result = ::close(this->ends[1]);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Right end of temporary pipe closed successfully");
    }
    if(this->ends[0] != -1) {
      int result = ::close(this->ends[0]);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Left end of temporary pipe closed successfully");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::CloseOneEnd(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int result = ::close(this->ends[whichEnd]);
    if(result != 0) [[unlikely]] {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not close one end of a pipe", errorNumber
      );
    }

    this->ends[whichEnd] = -1;
  }

  // ------------------------------------------------------------------------------------------- //

  int Pipe::ReleaseOneEnd(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int end = this->ends[whichEnd];
    this->ends[whichEnd] = -1;
    return end;
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::SetEndNonBlocking(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int result = ::fcntl(this->ends[whichEnd], F_GETFD);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not query file descriptor flags of a pipe end", errorNumber
      );
    }

    int newFlags = result | O_NONBLOCK;
    result = ::fcntl(this->ends[whichEnd], F_SETFD, newFlags);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not add O_NONBLOCK to the file descriptor flags of a pipe end", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::RequestProcessTermination(::pid_t processId){
    int result = ::kill(processId, SIGTERM);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not send SIGTERM to a process", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::KillProcess(::pid_t processId) {
    int result = ::kill(processId, SIGKILL);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not send SIGTERM to a process", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::GetOwnExecutablePath(std::filesystem::path &target) {
    getExecutablePath(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::GetAbsoluteExecutablePath(
    std::filesystem::path &target, const std::string &executable
  ) {
    if(PosixPathApi::IsPathRelative(executable)) {
      getExecutablePath(target);
      PosixPathApi::AppendPath(target, executable);
      if(PosixPathApi::DoesFileExist(target)) {
        return;
      }

      searchExecutableInPath(target, executable);
    } else {
      target.assign(executable);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::GetAbsoluteWorkingDirectory(
    std::filesystem::path &target, const std::string &workingDirectory
  ) {
    if(PosixPathApi::IsPathRelative(workingDirectory)) {
      getExecutablePath(target);
      PosixPathApi::AppendPath(target, workingDirectory);
    } else {
      target.assign(workingDirectory);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixProcessApi::searchExecutableInPath(
    std::filesystem::path &target, const std::string &executable
  ) {
    const char *path = ::getenv(u8"PATH");
    if(path != nullptr) {
      const char *start = path;
      while(*path != 0) {
        if(*path == ':') {
          if(path > start) {
            target.assign(start, path);
            PosixPathApi::AppendPath(target, executable);
            if(PosixPathApi::DoesFileExist(target)) {
              return;
            }
          }
          start = path + 1;
        }

        ++path;
      }

      // Final path in list.
      if(path > start) {
        target.assign(start, path);
        PosixPathApi::AppendPath(target, executable);
        if(PosixPathApi::DoesFileExist(target)) {
          return;
        }
      }
    }

    target.assign(executable);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
