#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Threading/Thread.h"

#if defined(NUCLEX_SUPPORT_WIN32) || defined(NUCLEX_SUPPORT_WINRT)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#include <time.h>
#endif

#include <thread> // for std::thread
#include <cstdlib> // for ldiv_t

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  void Thread::Sleep(std::chrono::microseconds time) {
#if defined(NUCLEX_SUPPORT_WIN32) || defined(NUCLEX_SUPPORT_WINRT)
    std::int64_t milliseconds = time.count();
    if(milliseconds > 0) {
      milliseconds += std::int64_t(500);
      milliseconds /= std::int64_t(1000);
    } else {
      return; // why does C++ even allow negative durations to exist?
    }

    ::Sleep(static_cast<DWORD>(milliseconds));
#elif defined(NUCLEX_SUPPORT_LINUX)
    timespec delay;
    {
      const long int MicrosecondsPerSecond = 1000000L;
      const long int NanosecondsPerMicrosecond = 1000L;

      ldiv_t result = ::ldiv(static_cast<long int>(time.count()), MicrosecondsPerSecond);
      delay.tv_sec = static_cast<time_t>(result.quot);
      delay.tv_nsec = static_cast<time_t>(result.rem * NanosecondsPerMicrosecond);
    }

    // CHECK: is nanosleep() the right choice?
    //   It can be interrupted by signals and return failure.
    ::nanosleep(&delay, nullptr);
#else
    std::this_thread::sleep_for(time);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  bool Thread::BelongsToThreadPool() {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
