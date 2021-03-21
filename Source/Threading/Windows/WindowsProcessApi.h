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

#ifndef NUCLEX_SUPPORT_THREADING_WINDOWS_WINDOWSPROCESSAPI_H
#define NUCLEX_SUPPORT_THREADING_WINDOWS_WINDOWSPROCESSAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "../../Helpers/WindowsApi.h"

#include <cassert>

namespace Nuclex { namespace Support { namespace Threading { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Directional pipe that can be used for inter-process communication</summary>
  class Pipe {
  
    /// <summary>Opens a new directional pipe</summary>
    /// <param name="securityAttributes">
    ///   Security attributes controlling whether the pipe is inherited to child processes
    /// </param>
    public: Pipe(const SECURITY_ATTRIBUTES &securityAttributes);

    /// <summary>Closes whatever end(s) of the pipe have not been used yet</summary>
    public: ~Pipe();

    /// <summary>Sets one end of the pipe to be a non-inheritable handle</summary>
    /// <param name="whichEnd">Which end of the pipe will become non-inheritable</param>
    public: void SetEndNonInheritable(std::size_t whichEnd);

    /// <summary>Closes one end of the pipe</summary>
    /// <param name="whichEnd">Which end of the pipe to close</param>
    public: void CloseOneEnd(std::size_t whichEnd);

    /// <summary>Relinquishes ownership of the handle for one end of the pipe</summary>
    /// <param name="whichEnd">For which end of the pipe ownership will be released</param>
    /// <returns>The handle of the relinquished end of the pipe</returns>
    public: HANDLE ReleaseOneEnd(std::size_t whichEnd);

    /// <summary>Fetches the handle of one end of the pipe</summary>
    /// <param name="whichEnd">
    ///   Index of the pipe end (0 or 1) whose handle will be returned
    /// </param>
    /// <returns>The handle of requested end of the pipe</returns>
    public: HANDLE GetOneEnd(std::size_t whichEnd) {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      return this->ends[whichEnd];
    }

    /// <summary>Handle for the readable and the writable end of the pipe</summary>
    /// <remarks>
    ///   Index 0 is the readable end of the pipe, 1 is the writable end
    /// </remarks>
    private: HANDLE ends[2];
  
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Windows process and inter-process communication API</summary>
  class WindowsProcessApi {

    /// <summary>Retrieves the exit code a process has exited with</summary>
    /// <param name="processHandle">Handle of the process whose exit code will be checked</param>
    /// <returns>
    ///   The exit code of the process or STILL_ACTIVE if the process has not exited yet
    /// </returns>
    public: static DWORD GetProcessExitCode(HANDLE processHandle);

    /// <summary>Obtains the full path of the specified module</summary>
    /// <param name="moduleHandle">
    ///   Handle of the module whose path will be determined, nullptr for executable
    /// </param>
    /// <returns>The full path to the specified module</returns>
    private: static std::wstring getModuleFileName(HMODULE moduleHandle = nullptr);

    /// <summary>
    ///   Determines the absolute path of an executable by checking the system's search paths
    /// </summary>
    /// <param name="target">Target string to store the executable path in</param>
    /// <param name="executable">Executable, with or without path</param>
    /// <remarks>
    ///   <para>
    ///     This simply wraps the SearchPath() method. A warning on MSDN states that this
    ///     method works differently from how LoadLibrary() searches paths, one of the differences
    ///     is that it doesn't look in the executable's own directory first.
    ///   </para>
    ///   <para>
    ///     However, if we want passing the executable as the zeroeth parameter in
    ///     CreateProcess() optional, we need to use the ModuleName argument which only
    ///     accepts the absolute, full path of an executable file.
    ///   </para>
    /// </remarks>
    private: static void searchExecutablePath(
      std::wstring &target, const std::wstring &executable
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_THREADING_WINDOWS_WINDOWSPROCESSAPI_H
