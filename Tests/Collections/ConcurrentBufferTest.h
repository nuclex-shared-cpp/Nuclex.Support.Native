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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H

#include <gtest/gtest.h>

#include "Nuclex/Support/BitTricks.h"

#include <vector> // for std::vector
#include <memory> // for std::unique_ptr
#include <thread> // for std::thread
#include <atomic> // for std::atomic
#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Base class that allows testing lock-free buffers under high contention</summary>
  /// <remarks>
  ///   <para>
  ///     The problem in actually forcing a buffer into a high contention situation is generally
  ///     to make the threads really run at the same time. Thread scheduling can introduce
  ///     millisecond delays and mutexes, too - depending on the OS used.
  ///   </para>
  ///   <para>
  ///     This class will put threads into a busy spin until all threads are confirmed running
  ///     and then have them set off all at the same time (synchronized lock-free and without
  ///     waiting on a mutex or similar synchronization primitive). This has a very decent
  ///     chance of making all threads bugger the buffer being tested at the same time right
  ///     from the get-go.
  ///   </para>
  /// </remarks>
  class HighContentionBufferTest {

    /// <summary>Initializes a new high contention buffer test</summary>
    /// <param name="threadCount">Number of threads that will run at the same time</param>
    public: HighContentionBufferTest(std::size_t threadCount) :
      threadCount(threadCount),
      threads(),
      allThreadsMask(getBitMaskForThreadCount(threadCount)),
      startSignals(0),
      constructionTime(std::chrono::high_resolution_clock::now()),
      startMicroseconds(0),
      endMicroseconds(0) {

      // If we don't have enough bits for the threads, our start signal will not work...
      assert(
        (threadCount < (sizeof(std::size_t) * 8)) &&
        u8"Number of threads tested does not exceed number of bits in std::size_t"
      );
    }

    /// <summary>Waits for all threads to complete when the test is terminated</summary>
    public: ~HighContentionBufferTest() {
      JoinThreads();
    }

    /// <summary>Starts all threads at the same time</summary>
    /// <remarks>
    ///   Call this after all other test preparations are complete.
    /// </remarks>
    public: void StartThreads();

    /// <summary>Waits for all threads to finish executing</summary>
    /// <remarks>
    ///   Call this if you want to retrieve test results. Note that this method does not
    ///   stop the threads, it merely waits for them to stop by themselves.
    /// </remarks>
    public: void JoinThreads();

    /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
    /// <returns>The elapsed number of microseconds</returns>
    public: std::size_t GetElapsedMicroseconds() const {
      // Better hope the high_resolution_clock was monotonic...
      assert(
        (this->endMicroseconds >= this->startMicroseconds) &&
        u8"std::chrono::high_resolution_clock counts monotonically"
      );
      return static_cast<std::size_t>(this->endMicroseconds - this->startMicroseconds);
    }

    /// <summary>Method that will be executed by many threads at the same time</summary>
    /// <param name="threadIndex">Zero-based index of the thread in the test</param>
    /// <remarks>
    ///   The thread index is simply a unique sequential number assigned to each thread.
    ///   It can be used if you want a set of threads to do something different than others.
    /// </remarks>
    protected: virtual void Thread(std::size_t threadIndex) {
      (void)threadIndex;
    }

    /// <summary>
    ///   Thread entry point, keeps each thread in a busy spin until all threads are ready
    /// </summary>
    /// <param name="threadIndex">Zero-based index of the thread in the test</param>
    private: void threadStarter(std::size_t threadIndex);

    /// <summary>Marks the benchmark starting time if this is the first call</summary>
    private: void markStartTime();

    /// <summary>Marks the benchmark ending time if this is the first call</summary>
    private: void markEndTime();

    /// <summary>Forms a bit mask where one bit is set for each thread</summary>
    /// <param name="threadCount">Number of threads for which bits should be set</param>
    /// <returns>A bit mask with sequential bits set one for each thread</returns>
    private: static std::size_t getBitMaskForThreadCount(std::size_t threadCount);

    /// <summary>Number of threads that will be involved in the test</summary>
    private: std::size_t threadCount;
    /// <summary>Threads that are being used to run the tests</summary>
    private: std::vector<std::unique_ptr<std::thread>> threads;

    /// <summary>Mask of bits for all threads</summary>
    private: std::size_t allThreadsMask;
    /// <summary>Used to make all threads start at the same time</summary>
    private: std::atomic<std::size_t> startSignals;

    /// <summary>Time at which the instance was constructed</summary>
    private: std::chrono::high_resolution_clock::time_point constructionTime;
    /// <summary>Recorded start time, in microseconds, for the benchmark</summary>
    private: std::atomic<std::chrono::microseconds::rep> startMicroseconds;
    /// <summary>Recorded end time, in microseconds, for the benchmark</summary>
    private: std::atomic<std::chrono::microseconds::rep> endMicroseconds;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmark that tests the performance of appending single items</summary>
  /// <typeparam name="TConcurrentBuffer">Buffer that will be used for the benchmark</typeparam>
  template<template<typename TItem> class TConcurrentBuffer>
  class BufferAppendBenchmark : public HighContentionBufferTest {

    public: const std::size_t BenchmarkedItemCount = 1048576 * 4; // 4 Million items

    /// <summary>Initializes a new single item append benchmark</summary>
    /// <param name="threadCount">Number of threads that will be hammering the buffer</param>
    public: BufferAppendBenchmark(std::size_t threadCount) :
      HighContentionBufferTest(threadCount),
      buffer(BenchmarkedItemCount),
      addedItemCount(0) {}

    /// <summary>Thread worker method, performs the work being benchmarked</summary>
    /// <param name="threadIndex">Index of the thread this method is running in</param>
    protected: void Thread(std::size_t threadIndex) override {
      std::size_t randomNumber = BitTricks::XorShiftRandom(threadIndex);
      for(;;) {
        std::size_t newAddedItemCount = this->addedItemCount.fetch_add(
          1, std::memory_order_consume // if() below carries dependency
        ) + 1;
        if(newAddedItemCount > BenchmarkedItemCount) {
          this->addedItemCount.fetch_sub(1, std::memory_order_relaxed); // decrement back
          break;
        }

        bool wasAdded = this->buffer.TryAppend(static_cast<int>(randomNumber | 1));
        EXPECT_TRUE(wasAdded);

        randomNumber = BitTricks::XorShiftRandom(randomNumber);
      }
    }

    /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
    /// <returns>The elapsed number of microseconds</returns>
    public: std::size_t CountAddedItems() const {
      return this->addedItemCount.load(std::memory_order_acquire);
    }

    /// <summary>Buffer that is being benchmarked</summary>
    private: TConcurrentBuffer<int> buffer;
    /// <summary>Number of items that have been added to the buffer</summary>
    private: std::atomic<std::size_t> addedItemCount;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmarks the single item append method of a concurrent buffer</summary>
  /// <typeparam name="TConcurrentBuffer">
  ///   Type of concurrent buffer that will be tested
  /// </typeparam>
  /// <param name="maximumThreadCount">Number of threads up to which to test</param>
  template<template<typename TItem> class TConcurrentBuffer>
  void benchmarkSingleItemAppends(
    std::size_t maximumThreadCount = std::thread::hardware_concurrency()
  ) {
    typedef BufferAppendBenchmark<TConcurrentBuffer> BenchmarkType;

    for(std::size_t threadCount = 1; threadCount <= maximumThreadCount; ++threadCount) {
      BenchmarkType bench(threadCount);
      bench.StartThreads();
      bench.JoinThreads();

      EXPECT_EQ(bench.CountAddedItems(), bench.BenchmarkedItemCount);

      double kitemsPerSecond = static_cast<double>(bench.CountAddedItems());
      kitemsPerSecond /= static_cast<double>(bench.GetElapsedMicroseconds());
      kitemsPerSecond *= static_cast<double>(1000.0); // items/microsecond -> kitems/second

      std::cout <<
        "Adding " << bench.BenchmarkedItemCount << " items " <<
        "from " << threadCount << " threads: " <<
        std::fixed << (static_cast<double>(bench.GetElapsedMicroseconds()) / 1000.0)  << " ms" <<
        " (" << std::fixed << kitemsPerSecond << "K ops/second)" <<
        std::endl;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmark that tests the performance of taking single items</summary>
  /// <typeparam name="TConcurrentBuffer">Buffer that will be used for the benchmark</typeparam>
  template<template<typename TItem> class TConcurrentBuffer>
  class BufferTakeBenchmark : public HighContentionBufferTest {

    public: const std::size_t BenchmarkedItemCount = 1048576 * 4; // 4 Million items

    /// <summary>Initializes a new single item append benchmark</summary>
    /// <param name="threadCount">Number of threads that will be hammering the buffer</param>
    public: BufferTakeBenchmark(std::size_t threadCount) :
      HighContentionBufferTest(threadCount),
      buffer(BenchmarkedItemCount),
      takenItemCount(0) {

      std::size_t randomNumber = BitTricks::XorShiftRandom(threadCount);
      for(std::size_t index = 0; index < BenchmarkedItemCount; ++index) {
        EXPECT_TRUE(this->buffer.TryAppend(static_cast<int>(randomNumber)));
        randomNumber = BitTricks::XorShiftRandom(randomNumber);
      }

      EXPECT_EQ(buffer.Count(), BenchmarkedItemCount);
    }

    /// <summary>Thread worker method, performs the work being benchmarked</summary>
    protected: void Thread(std::size_t) override {
      int value = 0;

      for(;;) {
        std::size_t newTakenItemCount = this->takenItemCount.fetch_add(
          1, std::memory_order_consume // if() below carries dependency
        ) + 1;
        if(newTakenItemCount > BenchmarkedItemCount) {
          this->takenItemCount.fetch_sub(1, std::memory_order_relaxed); // decrement back
          break;
        }

        bool wasTaken = this->buffer.TryTake(value);
        EXPECT_TRUE(wasTaken);
      }
    }

    /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
    /// <returns>The elapsed number of microseconds</returns>
    public: std::size_t CountTakenItems() const {
      return this->takenItemCount.load(std::memory_order_acquire);
    }

    /// <summary>Buffer that is being benchmarked</summary>
    private: TConcurrentBuffer<int> buffer;
    /// <summary>Number of items that have been taken from the buffer</summary>
    private: std::atomic<std::size_t> takenItemCount;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmarks the single item taking method of a concurrent buffer</summary>
  /// <typeparam name="TConcurrentBuffer">
  ///   Type of concurrent buffer that will be tested
  /// </typeparam>
  /// <param name="maximumThreadCount">Number of threads up to which to test</param>
  template<template<typename TItem> class TConcurrentBuffer>
  void benchmarkSingleItemTakes(
    std::size_t maximumThreadCount = std::thread::hardware_concurrency()
  ) {
    typedef BufferTakeBenchmark<TConcurrentBuffer> BenchmarkType;

    for(std::size_t threadCount = 1; threadCount <= maximumThreadCount; ++threadCount) {
      BenchmarkType bench(threadCount);
      bench.StartThreads();
      bench.JoinThreads();

      EXPECT_EQ(bench.CountTakenItems(), bench.BenchmarkedItemCount);

      double kitemsPerSecond = static_cast<double>(bench.CountTakenItems());
      kitemsPerSecond /= static_cast<double>(bench.GetElapsedMicroseconds());
      kitemsPerSecond *= static_cast<double>(1000.0); // items/microsecond -> kitems/second

      std::cout <<
        "Taking " << bench.BenchmarkedItemCount << " items " <<
        "from " << threadCount << " threads: " <<
        std::fixed << (static_cast<double>(bench.GetElapsedMicroseconds()) / 1000.0)  << " ms" <<
        " (" << std::fixed << kitemsPerSecond << "K ops/second)" <<
        std::endl;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmark that tests the performance of taking single items</summary>
  /// <typeparam name="TConcurrentBuffer">Buffer that will be used for the benchmark</typeparam>
  template<template<typename TItem> class TConcurrentBuffer>
  class BufferMixedBenchmark : public HighContentionBufferTest {

    public: const std::size_t BenchmarkedItemCount = 1048576 * 4; // 4 Million items

    /// <summary>Initializes a new single item append benchmark</summary>
    /// <param name="threadCount">Number of threads that will be hammering the buffer</param>
    public: BufferMixedBenchmark(std::size_t threadCount) :
      HighContentionBufferTest(threadCount),
      buffer(BenchmarkedItemCount / 4),
      operationCount(0) {

      // Pre-fill the buffer half-full so we don't benchmark a full adds or empty takes
      std::size_t randomNumber = BitTricks::XorShiftRandom(threadCount);
      for(std::size_t index = 0; index < BenchmarkedItemCount / 8; ++index) {
        EXPECT_TRUE(this->buffer.TryAppend(static_cast<int>(randomNumber)));
        randomNumber = BitTricks::XorShiftRandom(randomNumber);
      }

      EXPECT_EQ(buffer.Count(), BenchmarkedItemCount / 8);
    }

    /// <summary>Thread worker method, performs the work being benchmarked</summary>
    /// <param name="threadIndex">Index of the thread this method is running in</param>
    protected: void Thread(std::size_t threadIndex) override {
      if(threadIndex % 1 == 0) {
        std::size_t randomNumber = BitTricks::XorShiftRandom(threadIndex);
        for(;;) {
          std::size_t safeOperationCount = this->operationCount.fetch_add(
            1, std::memory_order_consume // if() below carries dependency
          ) + 1;
          if(safeOperationCount > BenchmarkedItemCount) {
            this->operationCount.fetch_sub(1, std::memory_order_release);
            break;
          }

          this->buffer.TryAppend(static_cast<int>(randomNumber | 1));

          randomNumber = BitTricks::XorShiftRandom(randomNumber);
        }
      } else {
        int value = 0;
        for(;;) {
          std::size_t safeOperationCount = this->operationCount.fetch_add(
            1, std::memory_order_consume // if() below carries dependency
          ) + 1;
          if(safeOperationCount > BenchmarkedItemCount) {
            this->operationCount.fetch_sub(1, std::memory_order_release);
            break;
          }

          this->buffer.TryTake(value);
        }
      }
    }

    /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
    /// <returns>The elapsed number of microseconds</returns>
    public: std::size_t CountOperations() const {
      return this->operationCount.load(std::memory_order_acquire);
    }

    /// <summary>Buffer that is being benchmarked</summary>
    private: TConcurrentBuffer<int> buffer;
    /// <summary>Number of items that have been added or taken from the buffer</summary>
    private: std::atomic<std::size_t> operationCount;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Benchmarks the single item adding and taking methods of a concurrent buffer
  /// </summary>
  /// <typeparam name="TConcurrentBuffer">
  ///   Type of concurrent buffer that will be tested
  /// </typeparam>
  /// <param name="maximumThreadCount">Number of threads up to which to test</param>
  template<template<typename TItem> class TConcurrentBuffer>
  void benchmarkSingleItemMixed(
    std::size_t maximumThreadCount = std::thread::hardware_concurrency()
  ) {
    typedef BufferMixedBenchmark<TConcurrentBuffer> BenchmarkType;

    for(std::size_t threadCount = 1; threadCount <= maximumThreadCount; ++threadCount) {
      BenchmarkType bench(threadCount);
      bench.StartThreads();
      bench.JoinThreads();

      EXPECT_GE(bench.CountOperations(), bench.BenchmarkedItemCount);
      EXPECT_LE(bench.CountOperations(), bench.BenchmarkedItemCount + threadCount);

      double kitemsPerSecond = static_cast<double>(bench.CountOperations());
      kitemsPerSecond /= static_cast<double>(bench.GetElapsedMicroseconds());
      kitemsPerSecond *= static_cast<double>(1000.0); // items/microsecond -> kitems/second

      std::cout <<
        "Mixed Adding/Taking " << bench.CountOperations() << " items " <<
        "from " << threadCount << " threads: " <<
        std::fixed << (static_cast<double>(bench.GetElapsedMicroseconds()) / 1000.0)  << " ms" <<
        " (" << std::fixed << kitemsPerSecond << "K ops/second)" <<
        std::endl;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H
