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

#include <gtest/gtest.h>

#include <thread>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WANT_USELESS_THREAD_ID_QUERY)
  TEST(ThreadTest, CanGetCurrentThreadId) {
    std::uintptr_t threadId = Thread::GetCurrentThreadId();
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, ThreadsCanSleepAccurately) {
    Thread::Sleep(std::chrono::microseconds(25000));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, ThreadHasNativeIdentifier) {
    std::uintptr_t threadId = Thread::GetCurrentThreadId();
    EXPECT_NE(threadId, std::uintptr_t(nullptr));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, IdentifierOfOtherThreadCanBeQueried) {
    std::uintptr_t myThreadId, otherThreadId;

    {
      std::thread otherThread(
        [&] { otherThreadId = Thread::GetCurrentThreadId(); }
      );
      myThreadId = Thread::GetCurrentThreadId();
      otherThread.join();
    }

    EXPECT_NE(myThreadId, std::uintptr_t(nullptr));
    EXPECT_NE(otherThreadId, std::uintptr_t(nullptr));

    // OS could still run both on same core. If this check starts failing often, disable.
    // If you have an antique single-core system running a C++17 compiler, disable (the system).
    EXPECT_NE(myThreadId, otherThreadId);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, CpuAffinityCanBeChecked) {

    // Build a mask where the affinity bits for all CPUs are set
    std::uint64_t allCpusAffinity = 0;
    {
      std::size_t cpuCount = std::thread::hardware_concurrency();
      for(std::size_t index = 0; index < cpuCount; ++index) {
        allCpusAffinity |= (1 << index);
      }
    }

    // Query this thread's CPU affinity. It should allow all CPUs to schedule.
    std::uintptr_t threadId = Thread::GetCurrentThreadId();
    std::uint64_t myCpuAffinity = Thread::GetCpuAffinityMask(threadId);

    EXPECT_EQ(myCpuAffinity, allCpusAffinity);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, CpuAffinityCanBeChanged) {
    std::uint64_t unchangedAffinity, changedAffinity;

    // Use CPU cores 3 and 4
    std::uint64_t testedAffinity = (3 << 2);

    {
      std::thread otherThread(
        [&] {
          std::uintptr_t myThreadId = Thread::GetCurrentThreadId();
          unchangedAffinity = Thread::GetCpuAffinityMask(myThreadId);
          Thread::SetCpuAffinityMask(myThreadId, testedAffinity);
          changedAffinity = Thread::GetCpuAffinityMask(myThreadId);
        }
      );
      otherThread.join();
    }

    EXPECT_NE(unchangedAffinity, testedAffinity);
    EXPECT_NE(unchangedAffinity, changedAffinity);
    EXPECT_EQ(changedAffinity, testedAffinity);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
