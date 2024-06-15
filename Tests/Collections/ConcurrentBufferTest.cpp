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

#include "ConcurrentBufferTest.h"
#include "Nuclex/Support/Threading/Thread.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Forms a bit mask where the specific number of bits are set</summary>
  /// <param name="lowestBitIndex">Zero-based index of the lowest bit that will be set</param>
  /// <param name="bitCount">Number of bits that will be set</param>
  /// <returns>A bit mask with the specified range of bits set
  std::size_t BitMask(std::size_t lowestBitIndex, std::size_t bitCount) {
    return (
      (static_cast<std::size_t>(-1) << static_cast<int>(lowestBitIndex + bitCount)) ^
      (static_cast<std::size_t>(-1) << static_cast<int>(lowestBitIndex))
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  void HighContentionBufferTest::StartThreads() {
    for(std::size_t index = 0; index < this->threadCount; ++index) {
      this->threads.push_back(
        std::make_unique<std::thread>(&HighContentionBufferTest::threadStarter, this, index)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void HighContentionBufferTest::JoinThreads() {
    for(std::size_t index = 0; index < this->threads.size(); ++index) {
      this->threads[index]->join();
    }

    this->threads.clear();
  }

  // ------------------------------------------------------------------------------------------- //

  void HighContentionBufferTest::threadStarter(std::size_t threadIndex) {

#if 0 // This would require all tests cap themselves to the number of hardware threads...
    // Change the CPU affinity so this thread runs on the specified CPU
    {
      std::uint64_t cpuAffinityMask = (std::uint64_t(1) << threadIndex);
      Nuclex::Support::Threading::Thread::SetCpuAffinityMask(cpuAffinityMask);
    }
#endif

    std::size_t runningThreadsMask = this->startSignals.fetch_or(
      (std::size_t(1) << threadIndex), std::memory_order_acq_rel
    );

    // Do a busy spin until all threads are ready to launch (yep, this whacks CPU
    // load to 100% on the core running this thread!)
    while((runningThreadsMask & this->allThreadsMask) != this->allThreadsMask) {
      runningThreadsMask = this->startSignals.load(std::memory_order_consume);
    }

    // All threads are confirmed to be in their busy spins and should very nearly
    // simultaneously have detected this, so begin the actual work
    markStartTime();
    Thread(threadIndex);
    markEndTime();
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Marks the benchmark starting time if this is the first call</summary>
  void HighContentionBufferTest::markStartTime() {
    std::chrono::microseconds::rep zero = 0;
    this->startMicroseconds.compare_exchange_strong(
      zero,
      std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - this->constructionTime
      ).count()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void HighContentionBufferTest::markEndTime() {
    std::chrono::microseconds::rep zero = 0;
    this->endMicroseconds.compare_exchange_strong(
      zero,
      std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - this->constructionTime
      ).count()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t HighContentionBufferTest::getBitMaskForThreadCount(std::size_t threadCount) {
    return BitMask(0, threadCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, BitMaskIsCalculatedCorrectly) {
    EXPECT_EQ(BitMask(0, 1), 1U);
    EXPECT_EQ(BitMask(0, 2), 3U);
    EXPECT_EQ(BitMask(0, 3), 7U);
    EXPECT_EQ(BitMask(0, 4), 15U);
    EXPECT_EQ(BitMask(0, 5), 31U);
    EXPECT_EQ(BitMask(0, 6), 63U);
    EXPECT_EQ(BitMask(0, 7), 127U);
    EXPECT_EQ(BitMask(0, 8), 255U);
    EXPECT_EQ(BitMask(0, 9), 511U);
    EXPECT_EQ(BitMask(0, 10), 1023U);
    EXPECT_EQ(BitMask(0, 11), 2047U);
    EXPECT_EQ(BitMask(0, 12), 4095U);
    EXPECT_EQ(BitMask(0, 13), 8191U);
    EXPECT_EQ(BitMask(0, 14), 16383U);
    EXPECT_EQ(BitMask(0, 15), 32767U);
    EXPECT_EQ(BitMask(0, 16), 65535U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpOneThread) {
    HighContentionBufferTest oneThread(1);
    oneThread.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpTwoThreads) {
    HighContentionBufferTest twoThreads(2);
    twoThreads.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpFourThreads) {
    HighContentionBufferTest fourThreads(4);
    fourThreads.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, AllThreadsActuallyRun) {
    class ThreadCountingTest : public HighContentionBufferTest {
      public: ThreadCountingTest() :
        HighContentionBufferTest(4),
        executedThreadCount(0) {}

      public: std::size_t CountExecutedThreads() const {
        return this->executedThreadCount.load(std::memory_order_acquire);
      }

      protected: void Thread(std::size_t) override {
        this->executedThreadCount.fetch_add(1, std::memory_order_relaxed);
      }

      private: std::atomic<std::size_t> executedThreadCount;
    };

    ThreadCountingTest fourThreads;
    fourThreads.StartThreads();
    fourThreads.JoinThreads();
    EXPECT_EQ(fourThreads.CountExecutedThreads(), 4U);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
