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

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/ScopeGuard.h"

#include <exception> // for std::runtime_error
#include <stdexcept> // for std::runtime_error (should be in <exception> but MSVC is weird)
#include <cassert> // for assert()

#include "../Helpers/WindowsApi.h"
#include <VersionHelpers.h> // for ::IsWindowsVistaOrGreater()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of logical processors in the system<summary>
  /// <returns>The number of logical processors available to the system</returns>
  std::size_t countLogicalProcessors() {
    ::SYSTEM_INFO systemInfo = { 0 };
    ::GetSystemInfo(&systemInfo); // There is no failure return...
    return static_cast<std::size_t>(systemInfo.dwNumberOfProcessors);
  }

#if 0

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Called by the thread pool to execute a work item</summary>
  /// <param name="parameter">Task the user has queued for execution</param>
  /// <returns>Always 0</returns>
  DWORD WINAPI threadPoolWorkCallback(void *parameter) {
    typedef std::function<void()> Task;
    typedef std::pair<Task, std::size_t> ReferenceCountedTask;

    ReferenceCountedTask *task = reinterpret_cast<ReferenceCountedTask *>(parameter);
    try {
      task->first.operator()();
    }
    catch(const std::exception &) {
      if(::InterlockedDecrement(&task->second) == 0) {
        delete task;
      }
      std::terminate();
    }

    if(::InterlockedDecrement(&task->second) == 0) {
      delete task;
    }
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Called by the thread pool to execute a work item</summary>
  /// <param name="context">Task the user has queued for execution</param>
  void NTAPI threadPoolWorkCallback(PTP_CALLBACK_INSTANCE, void *context, PTP_WORK) {
    typedef std::function<void()> Task;
    typedef std::pair<Task, std::size_t> ReferenceCountedTask;

    ReferenceCountedTask *task = reinterpret_cast<ReferenceCountedTask *>(context);
    try {
      task->first.operator()();
    }
    catch(const std::exception &) {
      if(::InterlockedDecrement(&task->second) == 0) {
        delete task;
      }

      // Termination is neccessary. If the ThreadPool worker fails, another part of
      // the application might wait forever on a mutex or never discover some data was
      // not processed. Make as much noise here as possible, then terminate the program!
      assert(!u8"Unhandled exception in ThreadPool worker thread. TERMINATING PROGRAM.");
      std::terminate();
      //std::unexpected();
    }

    if(::InterlockedDecrement(&task->second) == 0) {
      delete task;
    }
  }

  // ------------------------------------------------------------------------------------------- //
#endif
} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the process</summary>
    public: PlatformDependentImplementationData();
    /// <summary>Shuts down the thread pool and frees all resources it owns</summary>
    public: ~PlatformDependentImplementationData();

    /// <summary>Number of logical processors in the system</summary>
    public: std::size_t ProcessorCount; 
    /// <summary>Whether the thread pool should use the Vista-and-later API</summary>
    public: bool UseNewThreadPoolApi;
    /// <summary>Describes this application (WinSDK version etc.) to the thread pool</summary>
    public: ::TP_CALLBACK_ENVIRON NewCallbackEnvironment;
    /// <summary>Thread pool on which tasks get scheduled if new TP api is used</summary>
    public: ::TP_POOL *NewThreadPool;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementationData::PlatformDependentImplementationData() :
    ProcessorCount(countLogicalProcessors()),
    UseNewThreadPoolApi(::IsWindowsVistaOrGreater()),
    NewThreadPool(nullptr),
    NewCallbackEnvironment() {

    if(this->UseNewThreadPoolApi) {
      ::TpInitializeCallbackEnviron(&this->NewCallbackEnvironment);

      // Create a new thread pool. There is no documentation on how many threads it
      // will create or run by default.
      this->NewThreadPool = ::CreateThreadpool(nullptr);
      if(this->NewThreadPool == nullptr) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not create thread pool (using Vista and later API)", lastErrorCode
        );
      }
      {
        auto closeThreadPoolScope = ON_SCOPE_EXIT_TRANSACTION{
          ::CloseThreadpool(this->NewThreadPool);
        };

        // Set the minimum and maximum number of threads the thread pool can use.
        // Without doing this, we have no idea how many threads the thread pool would use.
        DWORD maximumThreadCount = static_cast<DWORD>(this->ProcessorCount * 2);
        ::SetThreadpoolThreadMaximum(this->NewThreadPool, maximumThreadCount);

        DWORD minimumThreadCount = static_cast<DWORD>(std::sqrt(this->ProcessorCount));
        if(minimumThreadCount < 4) {
          minimumThreadCount = 4;
        }
        BOOL result = ::SetThreadpoolThreadMinimum(this->NewThreadPool, minimumThreadCount);
        if(result == FALSE) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not set minimum number of thread pool threads", lastErrorCode
          );
        }

        // Connect the environment structure describing this application with
        // the thread pool. Needed to submit tasks to this pool instead of the default pool
        // (which probably gets created when the first task is submitted to it).
        ::SetThreadpoolCallbackPool(&this->NewCallbackEnvironment, this->NewThreadPool);
        // Another method without an error return

        closeThreadPoolScope.Commit();
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    if(this->UseNewThreadPoolApi) {
      ::CloseThreadpool(this->NewThreadPool);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool() :
    implementationData(new PlatformDependentImplementationData()) {}

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    delete this->implementationData;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::CountMaximumParallelTasks() const {
    return this->implementationData->ProcessorCount;
  }

#if 0

  // ------------------------------------------------------------------------------------------- //

  void WindowsThreadPool::AddTask(
    const std::function<void()> &task, std::size_t count /* = 1 */
  ) {
    typedef std::function<void()> Task;
    typedef std::pair<Task, std::size_t> ReferenceCountedTask;

    ReferenceCountedTask *countedTask = new ReferenceCountedTask(task, count);

    if(this->useNewThreadPoolApi) { // Vista and later can use the new API

      // Try to create a work item for the task we have been given
      PTP_WORK work = ::CreateThreadpoolWork(&threadPoolWorkCallback, countedTask, nullptr);
      if(work == nullptr) {
        delete countedTask;
        throw std::runtime_error("Could not create thread pool work item");
      }

      // Work item was created, submit it to the thread pool
      while(count > 0) {
        ::SubmitThreadpoolWork(work);
        --count;
      }

    } else { // We're on XP, so we need to use the old thread pool API

      // Queue all of the work items
      while(count > 0) {
        BOOL result = ::QueueUserWorkItem(
          &threadPoolWorkCallback, new std::function<void()>(task), 0
        );
        if(result == FALSE) {
          break;
        }

        --count;
      }

      // If we exited before all items were queued, an error has occurred
      if(count > 0) {
        while(count > 1) { // Some work items may already be running, can't assign
          ::InterlockedDecrement(&countedTask->second);
          --count;
        }

        // The final decrement may reveal that we're responsible for deleting the task
        // if all threads have finished their execution already.
        if(::InterlockedDecrement(&countedTask->second) == 0) {
          delete countedTask;
        }

        throw std::runtime_error("Could not queue thread pool work item");
      }

    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)
