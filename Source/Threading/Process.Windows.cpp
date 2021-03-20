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

#include "Nuclex/Support/Threading/Process.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Errors/TimeoutError.h"
#include "Nuclex/Support/Text/StringConverter.h"
#include "../Helpers/WindowsApi.h"

#include <exception> // for std::terminate()
#include <cassert> // for assert()

// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets up a pipe that can be used for inter-process communication</summary>
  class Pipe {
  
    /// <summary>Opens a new pipe</summary>
    /// <param name="securityAttributes">
    ///   Security attributes controlling whether the pipe is inherited to child processes
    /// </param>
    public: Pipe(const SECURITY_ATTRIBUTES &securityAttributes) :
      ends { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE } {

      // Create a new pipe. I am just drunkenly assuming that ::CreatePipe() does not
      // modify the SECURITY_ATTRIBUTES in any way because there are many code samples
      // where the same SECURITY_ATTRIBUTES instance is passed to several CreatePipe() calls.
      BOOL result = ::CreatePipe(
        &this->ends[0], &this->ends[1],
        const_cast<SECURITY_ATTRIBUTES *>(&securityAttributes), 0
      );
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not create temporary pipe", lastErrorCode
        );
      }
    }

    /// <summary>Sets one end of the pipe to be a non-inheritable handle</summary>
    /// <param name="whichEnd">Which end of the pipe will become non-inheritable</param>
    public: void SetEndNonInheritable(std::size_t whichEnd) {
      BOOL result = ::SetHandleInformation(this->ends[whichEnd], HANDLE_FLAG_INHERIT, 0);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not disable inheritability for pipe side", lastErrorCode
        );
      }
    }

    /// <summary>Closes one end of the pipe</summary>
    /// <param name="whichEnd">Which end of the pipe to close</param>
    public: void CloseOneEnd(std::size_t whichEnd) {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

      BOOL result = ::CloseHandle(this->ends[whichEnd]);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close one end of a pipe", lastErrorCode
        );
      }

      this->ends[whichEnd] = INVALID_HANDLE_VALUE;
    }

    /// <summary>Relinquishes ownership of the handle for one end of the pipe</summary>
    /// <param name="whichEnd">For which end of the pipe ownership will be released</param>
    /// <returns>The handle of the relinquished end of the pipe</returns>
    public: HANDLE ReleaseOneEnd(std::size_t whichEnd) {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      HANDLE end = this->ends[whichEnd];
      this->ends[whichEnd] = INVALID_HANDLE_VALUE;
      return end;
    }

    /// <summary>Fetches the handle of one end of the pipe</summary>
    /// <param name="whichEnd">
    ///   Index of the pipe end (0 or 1) whose handle will be returned
    /// </param>
    /// <returns>The handle of requested end of the pipe</returns>
    public: HANDLE GetOneEnd(std::size_t whichEnd) {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      return this->ends[whichEnd];
    }

    /// <summary>Closes whatever end(s) of the pipe have not been used yet</summary>
    public: ~Pipe() {
      if(this->ends[1] != INVALID_HANDLE_VALUE) {
        BOOL result = ::CloseHandle(this->ends[1]);
        assert((result != FALSE) && u8"Unused pipe side is successfully closed");
      }
      if(this->ends[0] != INVALID_HANDLE_VALUE) {
        BOOL result = ::CloseHandle(this->ends[0]);
        assert((result != FALSE) && u8"Unused pipe side is successfully closed");
      }
    }

    /// <summary>Handle for the readable and the writable end of the pipe</summary>
    private: HANDLE ends[2];
  
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Process::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the process</summary>
    public: PlatformDependentImplementationData() :
      ChildProcessHandle(INVALID_HANDLE_VALUE),
      StdinHandle(INVALID_HANDLE_VALUE),
      StdoutHandle(INVALID_HANDLE_VALUE),
      StderrHandle(INVALID_HANDLE_VALUE) {}

    /// <summary>Handle of the child process</summary>
    public: HANDLE ChildProcessHandle;
    /// <summary>File number of the writing end of the stdin pipe</summary>
    public: HANDLE StdinHandle;
    /// <summary>File number of the reading end of the stdout pipe</summary>
    public: HANDLE StdoutHandle;
    /// <summary>File numebr of the reading end of the stderr pipe</summary>
    public: HANDLE StderrHandle; 

  };

  // ------------------------------------------------------------------------------------------- //

  Process::Process(const std::string &executablePath) :
    executablePath(executablePath),
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // Things will still work, but we have to resort to an extra allocation.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      new(this->implementationDataBuffer) PlatformDependentImplementationData();
    } else {
      this->implementationData = new PlatformDependentImplementationData();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Process::~Process() {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle != INVALID_HANDLE_VALUE) {
      // TODO: Kill the child process
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Process::Start(
    const std::vector<std::string> &arguments /* = std::vector<std::string>() */,
    bool prependExecutableName /* = true */
  ) {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle != INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Child process is still running or not joined yet");
    }

    // Set up a security attribute structure that tells Windows that handles should
    // be inherited and use it when creating the pipes
    SECURITY_ATTRIBUTES pipeSecurityAttributes = {0};
    pipeSecurityAttributes.nLength = sizeof(pipeSecurityAttributes);
    pipeSecurityAttributes.bInheritHandle = TRUE; // non-default!
    pipeSecurityAttributes.lpSecurityDescriptor = nullptr;

    // Create the pipes and set the ends that belong to our side as non-inheritable
    Pipe stdinPipe(pipeSecurityAttributes);
    stdinPipe.SetEndNonInheritable(1);
    Pipe stdoutPipe(pipeSecurityAttributes);
    stdoutPipe.SetEndNonInheritable(0);
    Pipe stderrPipe(pipeSecurityAttributes);
    stderrPipe.SetEndNonInheritable(0);

    ::PROCESS_INFORMATION childProcessInfo = {0};
    // This one has no cbSize member...

    STARTUPINFO childProcessStartupSettings = {0};
    childProcessStartupSettings.cb = sizeof(childProcessStartupSettings);
    childProcessStartupSettings.hStdInput = stdinPipe.GetOneEnd(0);
    childProcessStartupSettings.hStdOutput = stdoutPipe.GetOneEnd(1);
    childProcessStartupSettings.hStdError = stderrPipe.GetOneEnd(1);
    childProcessStartupSettings.dwFlags = STARTF_USESTDHANDLES;

    // Launch the new process. We're using the UTF-16 version (and convert everything
    // from UTF-8 to UTF-16) to ensure we can deal with unicode paths and executable names.
    {
      std::wstring utf16ExecutablePath = Nuclex::Support::Text::StringConverter::WideFromUtf8(
        this->executablePath
      );
      BOOL result = ::CreateProcessW(
        nullptr, // application name -> figure it out yourself!
        utf16ExecutablePath.data(),
        nullptr, // use default security attributes
        nullptr, // use default thread security attributes
        TRUE, // yes, we want to inherit (some) handles
        0, // no extra creation flags
        nullptr, // use the current environment
        nullptr, // use our current directory,
        &childProcessStartupSettings,
        &childProcessInfo
      );
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not spawn new process", lastErrorCode
        );
      }
    }

    // Call was successful, the pipes now belong to the child process
    // CHECK: Does the child process have duplicates? Do I still have to close these?
    stdinPipe.CloseOneEnd(0); // stdinPipe.ReleaseOneEnd(0);
    stdoutPipe.CloseOneEnd(1); // stdoutPipe.ReleaseOneEnd(1);
    stderrPipe.CloseOneEnd(1); // stderrPipe.ReleaseOneEnd(1);

    // We don't need the handle to the main thread, but have ownership,
    // so be a good citizen and close it.
    {
      BOOL result = ::CloseHandle(childProcessInfo.hThread);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();

        result = ::CloseHandle(childProcessInfo.hProcess);
        assert((result != FALSE) && u8"Child process handle closed successfully");

        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close handle for child process main thread", lastErrorCode
        );
      }
    }

    impl.ChildProcessHandle = childProcessInfo.hProcess;
    impl.StdinHandle = stdinPipe.ReleaseOneEnd(1);
    impl.StdoutHandle = stdoutPipe.ReleaseOneEnd(0);
    impl.StderrHandle = stderrPipe.ReleaseOneEnd(0);
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::IsRunning() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      return false; // Not launched yet or joined already
    }

    DWORD exitCode;
    {
      BOOL result = ::GetExitCodeProcess(impl.ChildProcessHandle, &exitCode);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not check process exit code", lastErrorCode
        );
      }
    }

    // Well, the process may have exited with STILL_ACTIVE as its exit code :-(
    // Also check WaitForSingleObject() here...
    if(exitCode == STILL_ACTIVE) {
      DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, 0U);
      if(result == WAIT_OBJECT_0) {
        return false;
      } else if(result == WAIT_TIMEOUT) {
        return true;
      }

      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Error waiting for external process to exit", lastErrorCode
      );
    }
    
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Process::Wait(
    std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */
  ) const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      throw std::logic_error(u8"Process was not started or is already joined");
    }

    DWORD timeoutMilliseconds = static_cast<DWORD>(patience.count());
    DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, timeoutMilliseconds);
    if(result == WAIT_OBJECT_0) {
      return true;
    } else if(result == WAIT_TIMEOUT) {
      return false;
    }

    DWORD lastErrorCode = ::GetLastError();
    Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
      u8"Error waiting for external process to exit", lastErrorCode
    );
  }

  // ------------------------------------------------------------------------------------------- //

  int Process::Join(std::chrono::milliseconds patience /* = std::chrono::milliseconds(30000) */) {
    PlatformDependentImplementationData &impl = getImplementationData();
    if(impl.ChildProcessHandle == INVALID_HANDLE_VALUE) {
      return false; // Not launched yet or joined already
    }

    DWORD exitCode;
    {
      BOOL result = ::GetExitCodeProcess(impl.ChildProcessHandle, &exitCode);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not check process exit code", lastErrorCode
        );
      }
    }

    // Well, the process may have exited with STILL_ACTIVE as its exit code :-(
    // Also check WaitForSingleObject() here...
    if(exitCode == STILL_ACTIVE) {
      DWORD timeoutMilliseconds = static_cast<DWORD>(patience.count());
      DWORD result = ::WaitForSingleObject(impl.ChildProcessHandle, timeoutMilliseconds);
      if(result == WAIT_OBJECT_0) {
        impl.ChildProcessHandle = INVALID_HANDLE_VALUE;
        return exitCode; // Yep, someone returned STILL_ACTIVE as the process' exit code
      } else if(result == WAIT_TIMEOUT) {
        throw Nuclex::Support::Errors::TimeoutError(
          u8"Timed out waiting for external process to exit"
        );
      }

      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Error waiting for external process to exit", lastErrorCode
      );
    }

    // Process is well and truly done, close its process handle and clear our handle
    {
      BOOL result = ::CloseHandle(impl.ChildProcessHandle);
      if(result == FALSE) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not close handle of terminated child process", lastErrorCode
        );
      }

      impl.ChildProcessHandle = INVALID_HANDLE_VALUE;
    }

    return exitCode;
  }

  // ------------------------------------------------------------------------------------------- //

  const Process::PlatformDependentImplementationData &Process::getImplementationData() const {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<const PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Process::PlatformDependentImplementationData &Process::getImplementationData() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)