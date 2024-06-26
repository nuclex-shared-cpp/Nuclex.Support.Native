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

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)

#include "Nuclex/Support/Threading/Semaphore.h"
#include "Nuclex/Support/Threading/Thread.h"

#include "../Collections/ConcurrentBufferTest.h" // HighContentionBufferTest
#include "../../Source/Helpers/PosixApi.h" // PosixApi

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <thread> // for std::thread

#if !defined(NUCLEX_SUPPORT_WINDOWS)
#include <semaphore.h> // for ::sem_t, ::sem_init(), ::sem_post(), ::sem_wait()...
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Benchmark that repeatedly increments and waits on a ::sem_t</summary>
  class SemTBenchmark : public Nuclex::Support::Collections::HighContentionBufferTest {

    /// <summary>Initializes a new benchmark</summary>
    public: SemTBenchmark() :
      HighContentionBufferTest(std::thread::hardware_concurrency()),
      fullLockCount(std::thread::hardware_concurrency()),
      waitingLockCount(0),
      cycleCount(0) {

      int result = ::sem_init(&this->semaphore, 0, fullLockCount);
      if(result == -1) {
        int errorNumber = errno;
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"sem_init() failed", errorNumber
        );
      }
    }

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~SemTBenchmark() {
      int result = ::sem_destroy(&this->semaphore);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Semaphore is successfully destroyed");
    }

    /// <summary>
    ///   Increments the semaphore twice for each thread to launch the benchmark
    /// </summary>
    public: void KickOff() {
      for(std::size_t index = 0; index < this->fullLockCount * 2; ++index) {
        int result = ::sem_post(&this->semaphore);
        if(result == -1) {
          int errorNumber = errno;
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"sem_post() failed", errorNumber
          );
        }
      }
    }

    /// <summary>Executed by each thread simultaneously</summary>
    /// <param name="threadIndex">Unique index of the thread</param>
    protected: void Thread(std::size_t threadIndex) override {
      (void)threadIndex;

      for(;;) {

        // Check if the current cycle is complete. If so, kick off a new cycle
        {
          std::size_t safeLockCount = (
            this->waitingLockCount.fetch_add(1, std::memory_order_release) + 1
          );
          if(safeLockCount >= this->fullLockCount * 2) {
            this->waitingLockCount.store(0, std::memory_order_release);
            KickOff();
          }
        }

        // Pass through or wait on the semaphore (first loop passes through, second waits)
        int result = ::sem_wait(&this->semaphore);
        if(result == -1) {
          int errorNumber = errno;
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"sem_wait() failed", errorNumber
          );
        }

        // Increment the cycle count to stop the benchmark after a certain number of loops
        {
          std::size_t safeCycleCount = (
            this->cycleCount.fetch_add(1, std::memory_order_release) + 1
          );
          if(safeCycleCount >= 1000000) {
            break;
          }
        }

      } // for(;;)
    }

    /// <summary>Standard semaphore being tested</summary>
    private: ::sem_t semaphore;
    /// <summary>Lock count at which all threads would be waiting</summary>
    private: const std::size_t fullLockCount;
    /// <summary>Number of threads that have completed a loop</summary>
    private: std::atomic<std::size_t> waitingLockCount;
    /// <summary>Number of cycles the loop has completed between all threads</summary>
    private: std::atomic<std::size_t> cycleCount;

  };
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

  /// <summary>Benchmark that repeatedly increments and waits on a Nuclex semaphore</summary>
  class SemaphoreBenchmark : public Nuclex::Support::Collections::HighContentionBufferTest {

    /// <summary>Initializes a new benchmark</summary>
    public: SemaphoreBenchmark() :
      HighContentionBufferTest(std::thread::hardware_concurrency()),
      semaphore(std::thread::hardware_concurrency()),
      fullLockCount(std::thread::hardware_concurrency()),
      waitingLockCount(0),
      cycleCount(0) {}

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~SemaphoreBenchmark() {}

    /// <summary>
    ///   Increments the semaphore twice for each thread to launch the benchmark
    /// </summary>
    public: void KickOff() {
      this->semaphore.Post(this->fullLockCount * 2);
    }

    /// <summary>Executed by each thread simultaneously</summary>
    /// <param name="threadIndex">Unique index of the thread</param>
    protected: void Thread(std::size_t threadIndex) override {
      (void)threadIndex;

      for(;;) {

        // Check if the current cycle is complete. If so, kick off a new cycle
        {
          std::size_t safeLockCount = (
            this->waitingLockCount.fetch_add(1, std::memory_order_release) + 1
          );
          if(safeLockCount >= this->fullLockCount * 2) {
            this->waitingLockCount.store(0, std::memory_order_release);
            KickOff();
          }
        }

        // Pass through or wait on the semaphore (first loop passes through, second waits)
        this->semaphore.WaitThenDecrement();

        // Increment the cycle count to stop the benchmark after a certain number of loops
        {
          std::size_t safeCycleCount = (
            this->cycleCount.fetch_add(1, std::memory_order_release) + 1
          );
          if(safeCycleCount >= 1000000) {
            break;
          }
        }

      } // for(;;)
    }

    /// <summary>Standard semaphore being tested</summary>
    private: Nuclex::Support::Threading::Semaphore semaphore;
    /// <summary>Lock count at which all threads would be waiting</summary>
    private: const std::size_t fullLockCount;
    /// <summary>Number of threads that have completed a loop</summary>
    private: std::atomic<std::size_t> waitingLockCount;
    /// <summary>Number of cycles the loop has completed between all threads</summary>
    private: std::atomic<std::size_t> cycleCount;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  TEST(SemaphoreTest, SemTBenchmarkSucceeds) {
    SemTBenchmark bench;

    bench.StartThreads();
    bench.JoinThreads();

    std::cout <<
      "Running " << 1000000 << " cycles " <<
      "with " << std::thread::hardware_concurrency() << " threads: " <<
      std::fixed << (static_cast<double>(bench.GetElapsedMicroseconds()) / 1000.0)  << " ms" <<
      //" (" << std::fixed << kitemsPerSecond << "K ops/second)" <<
      std::endl;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, SemaphoreBenchmarkSucceeds) {
    SemaphoreBenchmark bench;

    bench.StartThreads();
    bench.JoinThreads();

    std::cout <<
      "Running " << 1000000 << " cycles " <<
      "with " << std::thread::hardware_concurrency() << " threads: " <<
      std::fixed << (static_cast<double>(bench.GetElapsedMicroseconds()) / 1000.0)  << " ms" <<
      //" (" << std::fixed << kitemsPerSecond << "K ops/second)" <<
      std::endl;

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)