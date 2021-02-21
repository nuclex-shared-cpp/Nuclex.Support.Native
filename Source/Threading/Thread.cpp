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

#if defined(NUCLEX_SUPPORT_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NO_MINMAX
#include <Windows.h> // for ::Sleep() and ::GetCurrentThreadId()
#elif defined(NUCLEX_SUPPORT_LINUX)
#include <ctime> // for ::clock_gettime() and ::clock_nanosleep()
#include <cstdlib> // for ldiv_t
#include <algorithm> // for std::min()
#include "../Helpers/PosixApi.h"
#endif

#include <thread> // for std::thread
#include <cassert> // for assert()

// Design: currently this does not take NUMA and >64 CPUs into account
//
// Reasoning:
//
// While there's potential to use many more cores than the average application in 2020 uses,
// use cases with more than 64 cores are usually found in AI and raytracing. I'm making the bet
// here that consumer CPUs will not grow above 64 cores for a decade.
//
// When dealing with more than 64 cores, additional precautions are needed for optimal
// performance, such as being aware of NUMA nodes (how many physical chips there are.).
// On Linux, this requires libnuma. On Windows, this is done via "processor groups".
// Microsoft's API also exposes up to 64 cores without processor group awareness.
//
// Counter arguments:
//
// If more than 64 cores become very common, applications built with Nuclex.Support.Native that
// make use of CPU affinity will all be hogging the lower 64 cores.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  void Thread::Sleep(std::chrono::microseconds time) {
#if defined(NUCLEX_SUPPORT_WIN32)
    std::int64_t milliseconds = time.count();
    if(milliseconds > 0) {
      milliseconds += std::int64_t(500);
      milliseconds /= std::int64_t(1000);
      ::Sleep(static_cast<DWORD>(milliseconds));
    }
#elif false && defined(NUCLEX_SUPPORT_LINUX) // Testing shows this to often not wait at all :-(
    const static long int MicrosecondsPerSecond = 1000000L;
    const static long int NanosecondsPerMicrosecond = 1000L;

    // Is the delay under 1 second? Use a simple relative sleep
    ::timespec endTime;

    // Get current time
    int result = ::clock_gettime(CLOCK_MONOTONIC, &endTime);
    if(result != 0) {
      //int errorNumber = errno;
      Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error retrieving current time via clock_gettime()", result
      );
    }

    // Calculate sleep end time
    {
      ldiv_t result = ::ldiv(static_cast<long int>(time.count()), MicrosecondsPerSecond);
      endTime.tv_sec += result.quot;
      endTime.tv_nsec += result.rem * NanosecondsPerMicrosecond;
      if(endTime.tv_nsec >= NanosecondsPerMicrosecond * MicrosecondsPerSecond) {
        endTime.tv_nsec -= NanosecondsPerMicrosecond * MicrosecondsPerSecond;
        endTime.tv_sec -= 1U;
      }
    }

    // Now attempt to sleep until the calculated point in time
    for(;;) {
      int result = ::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &endTime, nullptr);
      if(result == 0) {
        return;
      } else if(result != EINTR) {
        //int errorNumber = errno;
        Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Error pausing thread via ::clock_nanosleep()", result
        );
      }
    }
#else
    std::this_thread::sleep_for(time);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  bool Thread::BelongsToThreadPool() {
    return false; // Since we haven't implemented a thread pool yet, this is easy ;-)
  }

  // ------------------------------------------------------------------------------------------- //

  std::uintptr_t Thread::GetCurrentThreadId() {
#if defined(NUCLEX_SUPPORT_WIN32)
    DWORD currentThreadId = ::GetCurrentThreadId();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(DWORD)) &&
      u8"Windows thread id (DWORD) can be stored inside an std::uintptr_t"
    );

    std::uintptr_t result = 0;
    *reinterpret_cast<DWORD *>(&result) = GetCurrentThreadId;
    return result;

#else // LINUX and POSIX
    ::pthread_t currentThreadIdentity = ::pthread_self();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    std::uintptr_t result = 0;
    *reinterpret_cast<::pthread_t *>(&result) = currentThreadIdentity;
    return result;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::uintptr_t Thread::GetStdThreadId(std::thread &thread) {
#if defined(NUCLEX_SUPPORT_WIN32)
    DWORD threadId = thread.native_handle();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(DWORD)) &&
      u8"Windows thread id (DWORD) can be stored inside an std::uintptr_t"
    );

    std::uintptr_t result = 0;
    *reinterpret_cast<DWORD *>(&result) = threadId;
    return result;

#else // LINUX and POSIX
    ::pthread_t threadIdentity = thread.native_handle();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    std::uintptr_t result = 0;
    *reinterpret_cast<::pthread_t *>(&result) = threadIdentity;
    return result;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t Thread::GetCpuAffinityMask(std::uintptr_t threadId) {
#if defined(NUCLEX_SUPPORT_WIN32)
#else // LINUX and POSIX
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );
    ::pthread_t threadIdentifier = *reinterpret_cast<::pthread_t *>(&threadId);

    // Query the affinity into pthreads' cpu_set_t
    ::cpu_set_t cpuSet;
    //CPU_ZERO(&cpuSet);
    int errorNumber = ::pthread_getaffinity_np(threadIdentifier, sizeof(cpuSet), &cpuSet);
    if(errorNumber != 0) {
      Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error querying CPU affinity via pthread_getaffinity_np()", errorNumber
      );
    }

    // Now turn it into a bit mask
    std::uint64_t result = 0;
    {
      std::size_t maxCpuIndex = std::min(64, CPU_SETSIZE);
      for(std::size_t index = 0; index < maxCpuIndex; ++index) {
        if(CPU_ISSET(index, &cpuSet)) {
          result |= (1 << index);
        }
      }
    }

    return result;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void Thread::SetCpuAffinityMask(std::uintptr_t threadId, std::uint64_t affinityMask) {
#if defined(NUCLEX_SUPPORT_WIN32)
#else // LINUX and POSIX
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );
    ::pthread_t threadIdentifier = *reinterpret_cast<::pthread_t *>(&threadId);

    // Translate the affinity mask into cpu_set_t
    ::cpu_set_t cpuSet;
    {
      CPU_ZERO(&cpuSet);

      std::size_t maxCpuIndex = std::min(64, CPU_SETSIZE);
      for(std::size_t index = 0; index < maxCpuIndex; ++index) {
        if((affinityMask & (1 << index)) != 0) {
          CPU_SET(index, &cpuSet);
        }
      }
    }

    // Apply the affinity setting
    int errorNumber = ::pthread_setaffinity_np(threadIdentifier, sizeof(cpuSet), &cpuSet);
    if(errorNumber != 0) {
      Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error changing CPU affinity via pthread_setaffinity_np()", errorNumber
      );
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
