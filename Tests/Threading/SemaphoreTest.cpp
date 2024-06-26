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

#include "Nuclex/Support/Threading/Semaphore.h"
#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <thread> // for std::thread
#include <stdexcept> // for std::system_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a thread to test the behavior of the semaphore</summary>
  class TestThread {

    /// <summary>Initializes a new test thread checking the specified semaphore</summary>
    /// <param name="semaphore">Semaphore that the thread will be checking</param>
    public: TestThread(Nuclex::Support::Threading::Semaphore &semaphore) :
      semaphore(semaphore),
      thread(),
      semaphorePassed(false) {}

    /// <summary>Waits for the thread to end and destroys it</summary>
    public: ~TestThread() {
      this->semaphore.Post(64);
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

    /// <summary>Checks whether the test thread has passed through the semaphore</summary>
    public: bool HasPassed() const {
      return this->semaphorePassed.load(std::memory_order_acquire);
    }

    /// <summary>Method that runs in a thread to check the semaphore function</summary>
    private: void threadMethod() {
      this->semaphore.WaitThenDecrement();
      this->semaphorePassed.store(true, std::memory_order_release);
    }

    /// <summary>Semaphore that the test thread will attempt to pass</summary>
    private: Nuclex::Support::Threading::Semaphore &semaphore;
    /// <summary>Thread that will attempt to pass the gate</summary>
    private: std::thread thread;
    /// <summary>Set to true as soon as the thread has passed the semaphore</summary>
    private: std::atomic<bool> semaphorePassed;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Semaphore semaphore;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, CanBeIncremented) {
    Semaphore semaphore;
    semaphore.Post();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, ThreadCanPassIncrementedSemaphore) {
    Semaphore semaphore;
    semaphore.Post();

    TestThread test(semaphore);
    test.LaunchThread();
    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, ThreadWaitsBeforeZeroedSemaphore) {
    Semaphore semaphore;

    TestThread test(semaphore);
    test.LaunchThread();

    // Give the thread some time to pass. We can't wait for the thread to
    // reach the semaphore without building a race condition of our own,
    // so we'll just give it ample time to hit the semaphore.
    Thread::Sleep(std::chrono::microseconds(25000)); // 25 ms

    // Thread should still be waiting in front of the semaphore
    EXPECT_FALSE(test.HasPassed());

    semaphore.Post();

    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, WaitCanTimeOut) {
    Semaphore semaphore;

    bool hasPassed = semaphore.WaitForThenDecrement(
      std::chrono::microseconds(1000)
    );
    EXPECT_FALSE(hasPassed);

    semaphore.Post();

    hasPassed = semaphore.WaitForThenDecrement(
      std::chrono::microseconds(1000)
    );
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
