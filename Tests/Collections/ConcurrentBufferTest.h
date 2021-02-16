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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H

#include <gtest/gtest.h>

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
      allThreadsMask(getBitMaskForThreadCount(threadCount)),
      startSignals(0) {

      const std::size_t bitCountInStdSizeT = sizeof(std::size_t) * 8;

      // If we don't have enough bits for the threads, our start signal will not work...
      assert(
        (threadCount < bitCountInStdSizeT) &&
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
    public: void StartThreads() {
      for(std::size_t index = 0; index < this->threadCount; ++index) {
        this->threads.push_back(
          std::make_unique<std::thread>(&HighContentionBufferTest::threadStarter, this, index)
        );
      }
    }

    /// <summary>Waits for all threads to finish executing</summary>
    /// <remarks>
    ///   Call this if you want to retrieve test results. Note that this method does not
    ///   stop the threads, it merely waits for them to stop by themselves.
    /// </remarks>
    public: void JoinThreads() {
      for(std::size_t index = 0; index < this->threads.size(); ++index) {
        this->threads[index]->join();
      }

      this->threads.clear();
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
    private: void threadStarter(std::size_t threadIndex) {
      std::size_t runningThreadsMask = this->startSignals.fetch_or(
        (1 << threadIndex), std::memory_order_acq_rel
      );

      // Do a busy spin until all threads are ready to launch (yep, this whacks CPU
      // load to 100% on the core running this thread!)
      while((runningThreadsMask & this->allThreadsMask) != this->allThreadsMask) {
        runningThreadsMask = this->startSignals.load(std::memory_order_consume);
      }

      // All threads are confirmed to be in their busy spins and should very nearly
      // simultaneously have detected this, so begin the actual work
      Thread(threadIndex);
    }

    /// <summary>Forms a bit mask where one bit is set for each thread</summary>
    /// <param name="threadCount">Number of threads for which bits should be set</param>
    /// <returns>A bit mask with sequential bits set one for each thread</returns>
    private: static std::size_t getBitMaskForThreadCount(std::size_t threadCount);

    /// <summary>Number of threads that will be involved in the test</summary>
    private: std::size_t threadCount;
    /// <summary>Mask of bits for all threads</summary>
    private: std::size_t allThreadsMask;
    /// <summary>Used to make all threads start at the same time</summary>
    private: std::atomic<std::size_t> startSignals;
    /// <summary>Threads that are being used to run the tests</summary>
    private: std::vector<std::unique_ptr<std::thread>> threads;

  };

  // ------------------------------------------------------------------------------------------- //

  template<template<typename TItem> class TConcurrentBuffer>
  void benchmarkSingleItemAppends() {

    /// <summary>Benchmark that tests the performance of appending single items</summary>
    class Benchmark : public HighContentionBufferTest {

      /// <summary>Initializes a new single item append benchmark</summary>
      /// <param name="threadCount">Number of threads that will be hammering the buffer</param>
      public: Benchmark(std::size_t threadCount) :
        HighContentionBufferTest(threadCount),
        buffer(1048576 * 4),
        addedItemCount(0),
        constructionTime(std::chrono::high_resolution_clock::now()),
        startMicroseconds(0),
        endMicroseconds(0) {}

      /// <summary>Thread worker method, performs the work being benchmarked</summary>
      /// <param name="threadIndex">Index of the thread this method is running in</param>
      protected: void Thread(std::size_t threadIndex) override {
        markStartTime();

        // Fill items into the queue until it accepts no more items
        {
          std::size_t randomNumber = fastRandomNumber(threadIndex);
          for(;;) {
            bool wasAdded = this->buffer.TryAppend(randomNumber);
            this->addedItemCount.fetch_add(1, std::memory_order_relaxed);
            if(!wasAdded) {
              break;
            }
            randomNumber = fastRandomNumber(randomNumber);
          }
        }

        markEndTime();
      }

      /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
      /// <returns>The elapsed number of microseconds</returns>
      public: std::size_t GetElapsedMicroseconds() const {
        // Better hope the high_resolution_clock was monotonic...
        return this->endMicroseconds - this->startMicroseconds;
      }

      /// <summary>Number of microseconds that have elapsed during the benchmark</summary>
      /// <returns>The elapsed number of microseconds</returns>
      public: std::size_t CountAddedItems() const {
        return this->addedItemCount.load(std::memory_order_acquire);
      }

      /// <summary>Generates the first pseudo-random number following a fixed seed</summary>
      /// <param name="seed">Seed value, same seeds produce same pseudo-random numbers</param>
      /// <returns>The first random number that followed the specified seed</returns>
      /// <remarks>
      ///   In some implementations of the C++ standard library (*cough* MSVC *cough*),
      ///   std::default_random_engine has a substantial setup and/or processing time,
      ///   taking 30+ seconds on a modern CPU to generate 128 KiB of data. Since quality
      ///   of random numbers is not important here, we use this fast "Xor-Shift" generator.
      /// </remarks>
      private: static std::size_t fastRandomNumber(std::size_t seed) {
        seed ^= (seed << 21);
        seed ^= (seed >> 35);
        seed ^= (seed << 4);
        return seed;
      }

      /// <summary>Marks the benchmark starting time if this is the first call</summary>
      private: void markStartTime() {
        std::size_t zero = 0;
        this->startMicroseconds.compare_exchange_strong(
          zero,
          std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - this->constructionTime
          ).count()
        );
      }

      /// <summary>Marks the benchmark ending time if this is the first call</summary>
      private: void markEndTime() {
        std::size_t zero = 0;
        this->endMicroseconds.compare_exchange_strong(
          zero,
          std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - this->constructionTime
          ).count()
        );
      }

      /// <summary>Buffer that is being benchmarked</summary>
      private: TConcurrentBuffer<int> buffer;
      /// <summary>Number of items that have been added to the buffer</summary>
      private: std::atomic<std::size_t> addedItemCount;
      /// <summary>Time at which the instance was constructed</summary>
      private: std::chrono::high_resolution_clock::time_point constructionTime;
      /// <summary>Recorded start time, in microseconds, for the benchmark</summary>
      private: std::atomic<std::size_t> startMicroseconds;
      /// <summary>Recorded end time, in microseconds, for the benchmark</summary>
      private: std::atomic<std::size_t> endMicroseconds;

    };

    for(std::size_t threadCount = 1; threadCount <= 16; ++threadCount) {
      Benchmark bench(threadCount);
      bench.StartThreads();
      bench.JoinThreads();

      double itemsPerSecond = static_cast<double>(bench.CountAddedItems());
      itemsPerSecond /= static_cast<double>(bench.GetElapsedMicroseconds());
      itemsPerSecond *= static_cast<double>(1000000.0); // microseconds -> seconds

      std::cout <<
        "Adding 1048576 items " <<
        "from " << threadCount << " threads: " <<
        bench.GetElapsedMicroseconds() << " μs" <<
        " (" << std::fixed << itemsPerSecond << " adds/second)" <<
        std::endl;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTBUFFERTEST_H
