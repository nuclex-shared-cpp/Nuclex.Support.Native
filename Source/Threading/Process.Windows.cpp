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

#include "../Helpers/WindowsApi.h"

#include <exception> // for std::terminate()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  class Pipe {};

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

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)