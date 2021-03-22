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

#ifndef NUCLEX_SUPPORT_THREADING_POSIX_POSIXPROCESSAPI_H
#define NUCLEX_SUPPORT_THREADING_POSIX_POSIXPROCESSAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include "../../Helpers/PosixApi.h"

#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Threading { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets up a pipe that can be used for inter-process communication</summary>
  class Pipe {

    /// <summary>Opens a new pipe</summary>
    public: Pipe();

    /// <summary>Closes whatever end(s) of the pipe have not been used yet</summary>
    public: ~Pipe();

    /// <summary>Closes one end of the pipe</summary>
    /// <param name="whichEnd">Which end of the pipe to close</param>
    public: void CloseOneEnd(int whichEnd);

    /// <summary>Relinquishes ownership of the file number for one end of the pipe</summary>
    /// <param name="whichEnd">For which end of the pipe ownership will be released</param>
    /// <returns>The file number of the relinquished end of the pipe</returns>
    public: int ReleaseOneEnd(int whichEnd);

    /// <summary>Enabled non-blocking IO for one end of the pipe</summary>
    /// <param name="whichEnd">For which end non-blocking IO will be enabled</param>
    public: void SetEndNonBlocking(int whichEnd);

    /// <summary>Fetches the file number of one end of the pipe</summary>
    /// <param name="whichEnd">
    ///   Index of the pipe end (0 or 1) whose file number will be returned
    /// </param>
    /// <returns>The file number for the requested end of the pipe</returns>
    public: int GetOneEnd(int whichEnd) const {
      assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");
      return this->ends[whichEnd];
    }

    /// <summary>File numbers for each end of the pipe</summary>
    private: int ends[2];
   
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix process and inter-process communication API</summary>
  class PosixProcessApi {


  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Posix

#endif // !defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_THREADING_POSIX_POSIXPROCESSAPI_H
