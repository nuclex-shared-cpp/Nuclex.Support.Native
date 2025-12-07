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

#include "../../Source/Interop/PosixTimeApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, CanAddMillisecondsToTime) {
    struct ::timespec futureTime = PosixTimeApi::GetTimePlus(
      CLOCK_MONOTONIC, std::chrono::milliseconds(100)
    );

    // Obtain the current time *after* fetching the 'future' time.
    // This way we can check if the tested method really returns a time in the future.
    struct ::timespec currentTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &currentTime);
    ASSERT_NE(result, -1);

    bool isFutureTimeInFuture = (
      (futureTime.tv_sec > currentTime.tv_sec) ||
      (
        (futureTime.tv_sec == currentTime.tv_sec) &&
        (futureTime.tv_nsec > currentTime.tv_nsec)
      )
    );
    ASSERT_TRUE(isFutureTimeInFuture);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, AddedMillisecondsAreActuallyMilliseconds) {
    struct ::timespec futureTime = PosixTimeApi::GetTimePlus(
      CLOCK_MONOTONIC, std::chrono::milliseconds(12)
    );

    struct ::timespec currentTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &currentTime);
    ASSERT_NE(result, -1);

    EXPECT_TRUE(
      (futureTime.tv_sec == currentTime.tv_sec) ||
      (futureTime.tv_sec == currentTime.tv_sec + 1)
    );

    EXPECT_FALSE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, futureTime));
    Threading::Thread::Sleep(std::chrono::milliseconds(25));
    EXPECT_TRUE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, futureTime));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, CanAddMicrosecondsToTime) {
    struct ::timespec futureTime = PosixTimeApi::GetTimePlus(
      CLOCK_MONOTONIC, std::chrono::microseconds(150000)
    );

    // Obtain the current time *after* fetching the 'future' time.
    // This way we can check if the tested method really returns a time in the future.
    struct ::timespec currentTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &currentTime);
    ASSERT_NE(result, -1);

    bool isFutureTimeInFuture = (
      (futureTime.tv_sec > currentTime.tv_sec) ||
      (
        (futureTime.tv_sec == currentTime.tv_sec) &&
        (futureTime.tv_nsec > currentTime.tv_nsec)
      )
    );
    ASSERT_TRUE(isFutureTimeInFuture);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, AddedMicrosecondsAreActuallyMicroseconds) {
    struct ::timespec futureTime = PosixTimeApi::GetTimePlus(
      CLOCK_MONOTONIC, std::chrono::microseconds(12500)
    );

    struct ::timespec currentTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &currentTime);
    ASSERT_NE(result, -1);

    EXPECT_TRUE(
      (futureTime.tv_sec == currentTime.tv_sec) ||
      (futureTime.tv_sec == currentTime.tv_sec + 1)
    );

    EXPECT_FALSE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, futureTime));
    Threading::Thread::Sleep(std::chrono::milliseconds(25));
    EXPECT_TRUE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, futureTime));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, CanCalculateRemainingTimeout) {
    struct ::timespec startTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &startTime);
    ASSERT_NE(result, -1);

    const std::size_t timeoutMicroseconds = 123456; //45678;

    std::size_t lastRemainingMicroseconds = timeoutMicroseconds;
    for(;;) {
      struct ::timespec remainingTimeout = PosixTimeApi::GetRemainingTimeout(
        CLOCK_MONOTONIC, startTime, std::chrono::microseconds(timeoutMicroseconds)
      );

      std::size_t remainingMicroseconds = (
        (remainingTimeout.tv_sec * 1000000) + ((remainingTimeout.tv_nsec + 500) / 1000)
      );
      //std::cout << remainingMicroseconds << std::endl;
      EXPECT_LE(remainingMicroseconds, timeoutMicroseconds);
      EXPECT_LE(remainingMicroseconds, lastRemainingMicroseconds);

      lastRemainingMicroseconds = remainingMicroseconds;
      if((remainingTimeout.tv_sec == 0) && (remainingTimeout.tv_nsec == 0)) {
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixTimeApiTest, CanDetectTimeout) {
    struct ::timespec pastTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &pastTime);
    ASSERT_NE(result, -1);

    // Wait until the clock's reported time has changed. Once that happens,
    // the previously queried time is guaranteed to lie in the past.
    for(std::size_t spin = 0; spin < 1000000; ++spin) {
      struct ::timespec currentTime;
      int result = ::clock_gettime(CLOCK_MONOTONIC, &currentTime);
      ASSERT_NE(result, -1);
      if((currentTime.tv_sec != pastTime.tv_sec) || (currentTime.tv_nsec != pastTime.tv_nsec)) {
        break;
      }
    }

    // Also get a sample of a future point in time that is guaranteed to not have timed out
    struct ::timespec futureTime;
    futureTime = PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, std::chrono::milliseconds(100));

    ASSERT_TRUE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, pastTime));
    ASSERT_FALSE(PosixTimeApi::HasTimedOut(CLOCK_MONOTONIC, futureTime));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
