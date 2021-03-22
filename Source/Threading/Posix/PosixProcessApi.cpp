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

#include "PosixProcessApi.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include <unistd.h> // for ::pipe()
#include <fcntl.h> // for ::fcntl()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  Pipe::Pipe() :
    ends {-1, -1} {

    int result = ::pipe(this->ends);
    if(unlikely(result != 0)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not set up a pipe", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Pipe::~Pipe() {
    if(this->ends[1] != -1) {
      int result = ::close(this->ends[1]);
      assert((result == 0) && u8"Right end of temporary pipe closed successfully");
    }
    if(this->ends[0] != -1) {
      int result = ::close(this->ends[0]);
      assert((result == 0) && u8"Left end of temporary pipe closed successfully");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Pipe::CloseOneEnd(int whichEnd) {
    assert(((whichEnd == 0) || (whichEnd == 1)) && u8"whichEnd is either 0 or 1");

    int result = ::close(this->ends[whichEnd]);
    if(unlikely(result != 0)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
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
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not query file descriptor flags of a pipe end", errorNumber
      );
    }

    int newFlags = result | O_NONBLOCK;
    result = ::fcntl(this->ends[whichEnd], F_SETFD, newFlags);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not add O_NONBLOCK to the file descriptor flags of a pipe end", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Posix

#endif // !defined(NUCLEX_SUPPORT_WIN32)
