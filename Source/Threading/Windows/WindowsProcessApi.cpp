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

#include "WindowsProcessApi.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Text/StringConverter.h"
#include "../../Text/Utf8/checked.h"

#include <algorithm> // for std::max()
#include <cassert> // for assert()

#include <Shlwapi.h> // for ::PathIsRelativeW()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  Pipe::Pipe(const SECURITY_ATTRIBUTES &securityAttributes) :
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

  // ------------------------------------------------------------------------------------------- //

  Pipe::~Pipe() {
    if(this->ends[1] != INVALID_HANDLE_VALUE) {
      BOOL result = ::CloseHandle(this->ends[1]);
#if defined(NDEBUG)
      (void)result; // Without this VS2019 prints a compiler warning
#else
      assert((result != FALSE) && u8"Unused pipe side is successfully closed");
#endif
    }
    if(this->ends[0] != INVALID_HANDLE_VALUE) {
      BOOL result = ::CloseHandle(this->ends[0]);
#if defined(NDEBUG)
      (void)result; // Without this VS2019 prints a compiler warning
#else
      assert((result != FALSE) && u8"Unused pipe side is successfully closed");
#endif
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::SetEndNonInheritable(std::size_t whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    BOOL result = ::SetHandleInformation(this->ends[whichEnd], HANDLE_FLAG_INHERIT, 0);
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not disable inheritability for pipe side", lastErrorCode
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::CloseOneEnd(std::size_t whichEnd) {
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

  // ------------------------------------------------------------------------------------------- //

  HANDLE Pipe::ReleaseOneEnd(std::size_t whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
    HANDLE end = this->ends[whichEnd];
    this->ends[whichEnd] = INVALID_HANDLE_VALUE;
    return end;
  }

  // ------------------------------------------------------------------------------------------- //

  DWORD WindowsProcessApi::GetProcessExitCode(HANDLE processHandle) {
    DWORD exitCode;

    BOOL result = ::GetExitCodeProcess(processHandle, &exitCode);
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not check process exit code", lastErrorCode
      );
    }

    return exitCode;
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::GetAbsoluteExecutablePath(
    std::wstring &target, const std::wstring &executable
  ) {
    if(isPathRelative(executable)) {

      // Try the executable's own path
      {
        getModuleFileName(target);
        removeFileFromPath(target);
        appendPath(target, executable);
        if(doesFileExist(target)) {
          return;
        }
      }

      // Try the Windows system directory
      {
        getSystemDirectory(target);
        appendPath(target, executable);
        if(doesFileExist(target)) {
          return;
        }
      }

      // Try the Windows directory
      {
        getWindowsDirectory(target);
        appendPath(target, executable);
        if(doesFileExist(target)) {
          return;
        }
      }

      // Could test ::GetDllDirectoryW() here. Should we?

      // Finally, search the standard paths (PATH environment variable)
      {
        searchExecutablePath(target, executable);
        if(doesFileExist(target)) {
          return;
        }
      }

    }

    // Path was absolute or we failed to find the requested executable
    target.assign(executable);
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsProcessApi::doesFileExist(const std::wstring &path) {
    DWORD attributes = ::GetFileAttributesW(path.c_str());
    if(attributes == INVALID_FILE_ATTRIBUTES) {
      DWORD lastErrorCode = ::GetLastError();
      if(lastErrorCode == ERROR_FILE_NOT_FOUND) {
        return false;
      }

      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not check process exit code", lastErrorCode
      );
    }

    return (
      ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
      ((attributes & FILE_ATTRIBUTE_DEVICE) == 0)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsProcessApi::isPathRelative(const std::wstring &path) {
    BOOL result = ::PathIsRelativeW(path.c_str());
    return (result == TRUE); // For once, there is no error return
  }

  // ------------------------------------------------------------------------------------------- //
  
  void WindowsProcessApi::appendPath(std::wstring &path, const std::wstring &extra) {
    path.resize(MAX_PATH);
    BOOL result = ::PathAppendW(path.data(), extra.c_str());
    if(result == FALSE) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(u8"Could not append path", errorCode);
    }

    int pathLength = ::lstrlenW(path.c_str());
    path.resize(static_cast<std::size_t>(pathLength));
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::removeFileFromPath(std::wstring &path) {
    BOOL result = ::PathRemoveFileSpecW(path.data());
    if(result == TRUE) {
      int pathLength = ::lstrlenW(path.c_str());
      path.resize(static_cast<std::size_t>(pathLength));
    }
    // FALSE is no error return, it only states nothing was removed
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::getModuleFileName(
    std::wstring &target, HMODULE moduleHandle /* = nullptr */
  ) {
    target.resize(MAX_PATH);

    // Try to get the executable path with a buffer of MAX_PATH characters.
    DWORD result = ::GetModuleFileNameW(
      moduleHandle, target.data(), static_cast<DWORD>(target.size())
    );

    // As long the function returns the buffer size, it is indicating that the buffer
    // was too small. Keep enlarging the buffer by a factor of 2 until it fits.
    while(result == target.size()) {
      target.resize(target.size() * 2);
      result = ::GetModuleFileNameW(
        moduleHandle, target.data(), static_cast<DWORD>(target.size())
      );
    }

    // If the function returned 0, something went wrong
    if(unlikely(result == 0)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not determine executable module path");
      Helpers::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    target.resize(result);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::getSystemDirectory(std::wstring &target) {
    target.resize(MAX_PATH);

    UINT result = ::GetSystemDirectoryW(target.data(), MAX_PATH);
    if(result == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not get Windows system directory", errorCode
      );
    }

    target.resize(result);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::getWindowsDirectory(std::wstring &target) {
    target.resize(MAX_PATH);

    UINT result = ::GetWindowsDirectoryW(target.data(), MAX_PATH);
    if(result == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not get Windows directory", errorCode
      );
    }

    target.resize(result);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsProcessApi::searchExecutablePath(
    std::wstring &target, const std::wstring &executable
  ) {
    target.resize(MAX_PATH);

    DWORD characterCount;
    {
      // Let Windows search for the executable in the search paths
      {
        LPWSTR unusedFilePart;
        characterCount = ::SearchPathW(
          nullptr, executable.c_str(), L".exe", // .exe is only appended if no extension
          MAX_PATH, target.data(),
          &unusedFilePart // Is not declared optional, but we have no use for it
        );
      }

      // The method returns the number of characters written into the absolute path buffer.
      // If it returns 0, the executable could not be found (or something else went wrong).
      if(characterCount == 0) {
        DWORD lastErrorCode = ::GetLastError();

        std::string message;
        {
          static const std::string errorMessageBegin(u8"Could not locate executable '", 29);
          static const std::string errorMessageEnd(u8"' in standard search paths", 26);

          std::string utf8Executable = Nuclex::Support::Text::StringConverter::Utf8FromWide(
            executable
          );

          message.reserve(29 + utf8Executable.length() + 26 + 1);
          message.append(errorMessageBegin);
          message.append(utf8Executable);
          message.append(errorMessageEnd);
        }

        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          message, lastErrorCode
        );
      }
    }

    // Shrink the string to fit again. We don't use the actual shrink_to_fit() method here
    // because (special internal knowledge) the string will have all the command line
    // arguments appended to it next.
    target.resize(characterCount);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)



#if 0
std::wstring WindowsProcessApi::combinePaths(std::wstring &path, const std::wstring &extra) {
  std::wstring result;
  result.resize(std::max(std::size_t(MAX_PATH), path.length() + extra.length() + 1));

  LPWSTR combined = ::PathCombineW(result.data(), path.c_str(), extra.c_str());
  if(combined == nullptr) {
    DWORD errorCode = ::GetLastError();
    Helpers::WindowsApi::ThrowExceptionForSystemError(u8"Could not combine paths", errorCode);
  }

  int pathLength = ::lstrlenW(result.c_str());
  result.resize(static_cast<std::size_t>(pathLength));

  return result;
}
#endif