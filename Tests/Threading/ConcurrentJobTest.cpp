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

#include "Nuclex/Support/Threading/ConcurrentJob.h"
#include "Nuclex/Support/Threading/Latch.h"
#include "Nuclex/Support/Threading/StopToken.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock used to verify the behavior of the ConcurrentJob base class</summary>
  class ExampleJob : public Nuclex::Support::Threading::ConcurrentJob {

    /// <summary>Initializes a new example job</summary>
    public: ExampleJob() :
      ConcurrentJob(),
      RunCount(0),
      WasCanceled(false),
      WaitLatch(0),
      RunLatch(1),
      ThrowException(false) {}

    public: using ConcurrentJob::StartOrRestart;
    public: using ConcurrentJob::Join;
    public: using ConcurrentJob::Cancel;

    /// <summary>Called in the background thread to perform the actual work</summary>
    /// <param name="canceler">Token by which the operation can be signalled to cancel</param>
    protected: void DoWork(
      const std::shared_ptr<const Nuclex::Support::Threading::StopToken> &canceler
    ) override {

      // Increment the counters so the unit test can se that the job ran
      std::size_t runCount = ++this->RunCount;
      if(runCount == 1) {
        this->RunLatch.CountDown();
      }

      // If we're supposed to simulate a failure, do so
      if(this->ThrowException.load(std::memory_order::memory_order_acquire)) {
        throw std::length_error(u8"Dummy error");
      }

      // Wait 3 times 250 microseconds to avoid a race condition for when the unit test
      // wishes to test canceling a job while it is already running
      for(std::size_t index = 0; index < 10; ++index) {
        if(canceler->IsCanceled()) {
          this->WasCanceled.store(true, std::memory_order::memory_order_release);
          break;
        }
        this->WaitLatch.WaitFor(std::chrono::microseconds(250));
      }
    }

    /// <summary>How many times the job has been run</summary>
    public: std::atomic<std::size_t> RunCount;
    /// <summary>Whether the job was canceled</summary>
    public: std::atomic<bool> WasCanceled;
    /// <summary>Whether an exception should be thrown in the worker thread</summary>
    public: std::atomic<bool> ThrowException;
    /// <summary>Latch on which the thread will wait for cancelation</summary>
    public: Nuclex::Support::Threading::Latch WaitLatch;
    /// <summary>Latch the worker will set to let the unit test wait until it runs</summary>
    public: Nuclex::Support::Threading::Latch RunLatch;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, JobsCanBeCreated) {
    ASSERT_NO_THROW(
      ExampleJob test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, UnstartedJobsCanBeJoined) {
    ASSERT_NO_THROW(
      ExampleJob test;
      test.Join();
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, JobsCanBeExecuted) {
    ExampleJob test;
    test.StartOrRestart();
    test.Join();
    
    EXPECT_EQ(test.RunCount.load(std::memory_order::memory_order_acquire), 1U);
    EXPECT_FALSE(test.WasCanceled.load(std::memory_order::memory_order_acquire));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, JobsCanBeCancelled) {
    ExampleJob test;
    test.WaitLatch.Post(); // lock the latch

    test.StartOrRestart();
    bool wasRunning = test.RunLatch.WaitFor(std::chrono::microseconds(300));
    test.Cancel();
    test.Join();

    EXPECT_TRUE(wasRunning);
    EXPECT_EQ(test.RunCount.load(std::memory_order::memory_order_acquire), 1U);
    EXPECT_TRUE(test.WasCanceled.load(std::memory_order::memory_order_acquire));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, JobsCanBeRepeated) {
    ExampleJob test;
    test.WaitLatch.Post(); // lock the latch

    test.StartOrRestart();
    bool wasRunning = test.RunLatch.WaitFor(std::chrono::microseconds(300));
    test.StartOrRestart();
    test.WaitLatch.CountDown();
    test.Join();

    EXPECT_TRUE(wasRunning);
    EXPECT_EQ(test.RunCount.load(std::memory_order::memory_order_acquire), 2U);
    EXPECT_TRUE(test.WasCanceled.load(std::memory_order::memory_order_acquire));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentJobTest, ExceptionsAreRethrownInJoin) {
    ExampleJob test;
    test.ThrowException.store(true, std::memory_order::memory_order_release);

    test.StartOrRestart();
    EXPECT_THROW(
      test.Join(),
      std::length_error
    );

    EXPECT_EQ(test.RunCount.load(std::memory_order::memory_order_acquire), 1U);
    EXPECT_FALSE(test.WasCanceled.load(std::memory_order::memory_order_acquire));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
