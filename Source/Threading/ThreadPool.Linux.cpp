#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#include "Nuclex/Support/Threading/ThreadPool.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "Nuclex/Support/ScopeGuard.h" // for ScopeGuard

#include <cassert> // for assert()
#include <atomic> // for std::atomic
#include <thread> // for std::thread
#include <cmath> // for std::sqrt()

#include "../Helpers/PosixApi.h"
#include <sys/sysinfo.h> // for ::get_nprocs()
#include <semaphore.h> // for ::sem_init(), ::sem_wait(), ::sem_post(), ::sem_destroy()

// There is no OS-provided thread pool on Linux systems
//
// Thus, an entire stand-alone is implemented in the private implementation data
// class, invisible to the header. Which makes this file quite a bit larger than
// the Windows implementation which relies on an already existing implementation.
//
// Ideally, I'd want to spin up additional threads only if all threads are busy
// (up to 2x processors). This would provide some protection against misuse by
// running longer operations in the thread pool.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementation {

    /// <summary>Creates an instance of the platform dependent data container</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    /// <returns>The new data container instance</returns>
    /// <remarks>
    ///   This will result in a vanilla instance. The trickery you see in the code
    ///   is just to do one big heap allocation for both the data container and
    ///   the std::thread array (which gets put directly after in memory).
    /// </remarks>
    public: static PlatformDependentImplementation *CreateInstance(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );

    /// <summary>Destroys an instance of the platform dependent data container</summary>
    /// <param name="instance">Instance that will be destroyed</param>
    public: static void DestroyInstance(PlatformDependentImplementation *instance);

    /// <summary>Initializes a platform dependent data members of the process</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    protected: PlatformDependentImplementation(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );

    /// <summary>Destroys the resources owned by the platform dependent data container</summary>
    protected: ~PlatformDependentImplementation();

    /// <summary>Adds another thread to the pool</summary>
    /// <returns>True if the thread was added, false if the pool was full</returns>
    public: bool AddThread();

    private: void threadWorkerMethod();

    /// <summary>Minimum number of threads to always keep running</summary>
    public: std::size_t MinimumThreadCount; 
    /// <summary>Maximum number of threads to create under high load</summary>
    public: std::size_t MaximumThreadCount; 
    /// <summary>Semaphore that allows one thread for each task to pass</summary>
    public: ::sem_t TaskSemaphore;
    /// <summary>Whether the thread pool is in the process of shutting down</summary>
    public: std::atomic<bool> IsShuttingDown;
    /// <summary>Number of threads currently running</summary>
    public: std::atomic<std::size_t> ThreadCount;
    /// <summary>Number of thread slots currently occupied</summary>
    public: std::atomic<std::size_t> OccupiedCount;
    /// <summary>Running threads, capacity is always ProcessorCount * 2</summary>
    public: std::thread *Threads;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation *
  ThreadPool::PlatformDependentImplementation::CreateInstance(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) {
    std::size_t requiredByteCount = (
      sizeof(PlatformDependentImplementation) +
      (sizeof(std::thread[2]) * maximumThreadCount / 2)
    );

    // Allocate memory, perform in-place construction and use the extra memory
    // as the address for the std::thread array
    std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[requiredByteCount]);

    PlatformDependentImplementation *instance = (
      new(buffer.get()) PlatformDependentImplementation(minimumThreadCount, maximumThreadCount)
    );

    instance->Threads = reinterpret_cast<std::thread *>(
      buffer.release() + sizeof(PlatformDependentImplementation)
    );

    return instance;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::DestroyInstance(
    PlatformDependentImplementation *instance
  ) {
    instance->~PlatformDependentImplementation();
    delete[] reinterpret_cast<std::uint8_t *>(instance);
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::PlatformDependentImplementation(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) :
    MinimumThreadCount(minimumThreadCount),
    MaximumThreadCount(maximumThreadCount),
    TaskSemaphore {0},
    IsShuttingDown(false),
    ThreadCount(0),
    OccupiedCount(0),
    Threads(nullptr) {

    // Create a semaphore we use to let the required number of worker threads through
    int result = ::sem_init(&this->TaskSemaphore, 0, 0);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not create a new semaphore", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::~PlatformDependentImplementation() {

    // Safety check, if this assertion triggers you'll send all threads into segfaults.
#if !defined(NDEBUG)
    std::size_t remainingThreadCount = this->ThreadCount.load(
      std::memory_order::memory_order_relaxed
    );
    assert(
      (remainingThreadCount == 0) && u8"All threads have terminated before destruction"
    );
#endif

    // Kill the semaphore
    int result = ::sem_destroy(&this->TaskSemaphore);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != -1) && u8"Semaphore is successfully destroyed");

  }

  // ------------------------------------------------------------------------------------------- //

  bool ThreadPool::PlatformDependentImplementation::AddThread() {

    // Do not add new threads if the thread pool is shutting down. The thread pool
    // will wait for all threads to exit and then start destroying them, so at that
    // point, messing around in the thread array would lead to disaster.
    {
      bool isShuttingDown = this->IsShuttingDown.load(
        std::memory_order::memory_order_relaxed
      );
      if(isShuttingDown) {
        return false;
      }
    }

    // Blindly increment the number of occupied slots. If we find out that no space
    // was left, decrement again. Otherwise, set up the new thread and increase
    // the tracked thread count for real.
    std::size_t freeSlotIndex = this->OccupiedCount.fetch_add(
      1, std::memory_order::memory_order_acquire
    );
    {
      auto returnSlotScope = ON_SCOPE_EXIT_TRANSACTION {
        this->OccupiedCount.fetch_sub(1, std::memory_order::memory_order_release);
      };
      if(freeSlotIndex >= this->MaximumThreadCount) {
        return false;
      }

      // Looks like there was room for another thread, start it up
      new(this->Threads + freeSlotIndex) std::thread(
        &PlatformDependentImplementation::threadWorkerMethod, this
      );

      returnSlotScope.Commit();
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::threadWorkerMethod() {
    
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMinimumThreadCount() {
    std::size_t processorCountSquareRoot = static_cast<std::size_t>(
      std::sqrt(::get_nprocs()) + 0.5
    );
    if(processorCountSquareRoot >= 4) {
      return processorCountSquareRoot;
    } else {
      return 4;
    }
  }        

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMaximumThreadCount() {
    return static_cast<std::size_t>(::get_nprocs()) * 2;
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool(
    std::size_t minimumThreadCount /* = GetDefaultMinimumThreadCount() */,
    std::size_t maximumThreadCount /* = GetDefaultMaximumThreadCount() */
  ) :
    implementation(
      PlatformDependentImplementation::CreateInstance(minimumThreadCount, maximumThreadCount)
    ) {

    auto destroyImplementationScope = ON_SCOPE_EXIT_TRANSACTION {
      PlatformDependentImplementation::DestroyInstance(this->implementation);
    };
    for(std::size_t index = 0; index < minimumThreadCount; ++index) {
      this->implementation->AddThread();
    }
    destroyImplementationScope.Commit();

  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    this->implementation->IsShuttingDown.store(
      true, std::memory_order::memory_order_release
    );
    // TODO: Place shutdown code here

    PlatformDependentImplementation::DestroyInstance(this->implementation);
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t *ThreadPool::getOrCreateTaskMemory(std::size_t payload) {
    return new std::uint8_t[payload];
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::submitTask(Task *task) {

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX)
