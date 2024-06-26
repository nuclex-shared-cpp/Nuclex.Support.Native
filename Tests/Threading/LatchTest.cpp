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

#include "Nuclex/Support/Threading/Latch.h"
#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <thread> // for std::thread
#include <stdexcept> // for std::system_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a thread to test the behavior of the semaphore</summary>
  class TestThread {

    /// <summary>Initializes a new test thread checking the specified latch</summary>
    /// <param name="latch">Latch that the thread will be checking</param>
    public: TestThread(Nuclex::Support::Threading::Latch &latch) :
      latch(latch),
      thread(),
      latchPassed(false) {}

    /// <summary>Waits for the thread to end an destroys it</summary>
    public: ~TestThread() {
      if(this->thread.joinable()) {
        this->thread.join();
      }
    }

    /// <summary>Launches the test thread</summary>
    public: void LaunchThread() {
      if(this->thread.joinable()) {
        this->thread.join();
      }

      std::thread newThread(&TestThread::threadMethod, this);
      this->thread.swap(newThread);
    }

    /// <summary>Waits for the test thread to terminate</summary>
    public: void JoinThread() {
      this->thread.join();
    }

    /// <summary>Checks whether the test thread has passed through the latch</summary>
    public: bool HasPassed() const {
      return this->latchPassed.load(std::memory_order_acquire);
    }

    /// <summary>Method that runs in a thread to check the latch function</summary>
    private: void threadMethod() {
      this->latch.Wait();
      this->latchPassed.store(true, std::memory_order_release);
    }

    /// <summary>Latch that the test thread will attempt to pass</summary>
    private: Nuclex::Support::Threading::Latch &latch;
    /// <summary>Thread that will attempt to pass the gate</summary>
    private: std::thread thread;
    /// <summary>Set to true as soon as the thread has passed the latch</summary>
    private: std::atomic<bool> latchPassed;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Latch latch;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, CanBeIncremented) {
    Latch latch;
    latch.Post();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, ThreadCanPassZeroLatch) {
    Latch latch;

    TestThread test(latch);
    test.LaunchThread();
    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, ThreadWaitsBeforeIncrementedLatch) {
    Latch latch;
    latch.Post();

    TestThread test(latch);
    test.LaunchThread();

    // Give the thread some time to pass. We can't wait for the thread to
    // reach the latch without building a race condition of our own,
    // so we'll just give it ample time to hit the latch.
    Thread::Sleep(std::chrono::microseconds(25000)); // 25 ms

    // Thread should still be waiting in front of the latch
    EXPECT_FALSE(test.HasPassed());

    latch.CountDown();

    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, WaitCanTimeOut) {
    Latch latch;
    latch.Post();

    bool hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_FALSE(hasPassed);

    latch.CountDown();

    hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, LatchHonorsInitialCount) {
    Latch latch(2);

    latch.CountDown();

    bool hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_FALSE(hasPassed);

    latch.CountDown();

    hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LatchTest, CountdownAndPostHonorCount) {
    Latch latch;

    latch.Post(2);
    latch.CountDown();

    bool hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_FALSE(hasPassed);

    latch.Post();
    latch.CountDown(2);

    hasPassed = latch.WaitFor(
      std::chrono::microseconds(1000)
    );
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
