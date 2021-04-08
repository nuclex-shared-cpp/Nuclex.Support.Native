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

#include "Nuclex/Support/ScopeGuard.h" // for ScopeGuard
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

#include "ThreadPoolTaskPool.h" // thread pool settings + task pool
#include "../Helpers/WindowsApi.h" // error handling helpers

#include <cassert> // for assert()
#include <atomic> // for std;:atomic
#include <mutex> // for std;:mutex

#include <VersionHelpers.h> // for ::IsWindowsVistaOrGreater()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of logical processors in the system<summary>
  /// <returns>The number of logical processors available to the system</returns>
  std::size_t countLogicalProcessors() {
    static ::SYSTEM_INFO systemInfo = { 0 };
    if(systemInfo.dwNumberOfProcessors == 0) {
      ::GetSystemInfo(&systemInfo); // There is no failure return...
    }
    return static_cast<std::size_t>(systemInfo.dwNumberOfProcessors);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementation {

    #pragma region struct SubmittedTask

    /// <summary>Wraps a callback that can be schuled on a worker thread</summary>
    public: struct SubmittedTask {

      /// <summary>Initializes a new submitted task</summary>
      public: SubmittedTask() : Implementation(nullptr) {}

      /// <summary>
      ///   Destroys the thread pool work item then the task is no longer being recycled
      /// </summary>
      public: ~SubmittedTask() {
        ::CloseThreadpoolWork(this->Work);
      }

      /// <summary>Size of the payload allocated for this task instance</summary>
      public: std::size_t PayloadSize;
      /// <summary>
      ///   The instance of the PlatformDependentimplementation class that owns this task
      /// </summary>
      public: PlatformDependentImplementation *Implementation;
      /// <summary>The thread pool work item, if the new thread pool API is used</summary>
      public: ::TP_WORK *Work;
      // <summary>The task instance living in the payload</summary>
      public: ThreadPool::Task *Task;
      /// <summary>This contains a ThreadPool::Task (actually a derived type)</summary>
      public: std::uint8_t Payload[sizeof(std::intptr_t)];

    };

    #pragma endregion // SubmittedTask

    /// <summary>Initializes a platform dependent data members of the process</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    public: PlatformDependentImplementation(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );
    /// <summary>Shuts down the thread pool and frees all resources it owns</summary>
    public: ~PlatformDependentImplementation();

    /// <summary>Called by the thread pool to execute a work item</summary>
    /// <param name="parameter">Task the user has queued for execution</param>
    /// <returns>Always 0</returns>
    public: static DWORD WINAPI oldThreadPoolWorkCallback(void *parameter);

    /// <summary>Called by the thread pool to execute a work item</summary>
    /// <param name="instance">
    ///   Lets the callback request additional actions from the thread pool
    /// </param>
    /// <param name="context">Task the user has queued for execution</param>
    /// <param name="workItem">Work item for which this callback was invoked</param>
    public: static void NTAPI newThreadPoolWorkCallback(
      ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
    );

    /// <summary>Whether the thread pool is shutting down</summary>
    public: std::atomic<bool> IsShuttingDown;
    /// <summary>Whether the thread pool should use the Vista-and-later API</summary>
    public: bool UseNewThreadPoolApi;
    /// <summary>Describes this application (WinSDK version etc.) to the thread pool</summary>
    public: ::TP_CALLBACK_ENVIRON NewCallbackEnvironment;
    /// <summary>Thread pool on which tasks get scheduled if new TP api is used</summary>
    public: ::TP_POOL *NewThreadPool;
    /// <summary>Number of tasks that should currently be waiting in the thread pool</summary>
    public: std::atomic<std::size_t> ScheduledTaskCount;
    // TODO: This is an illegal use of the mutex class. Maybe use a condition_variable here?
    /// <summary>Signaled by the last thread shutting down</summary>
    public: std::timed_mutex ShutdownMutex;
    /// <summary>Submitted tasks for re-use</summary>
    public: ThreadPoolTaskPool<
      SubmittedTask, offsetof(SubmittedTask, Payload)
    > SubmittedTaskPool;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::PlatformDependentImplementation(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) :
    IsShuttingDown(false),
    UseNewThreadPoolApi(::IsWindowsVistaOrGreater()),
    NewCallbackEnvironment(),
    NewThreadPool(nullptr),
    ScheduledTaskCount(0),
    ShutdownMutex(),
    SubmittedTaskPool() {

    if(this->UseNewThreadPoolApi) {
      ::TpInitializeCallbackEnviron(&this->NewCallbackEnvironment);

      // Create a new thread pool. There is no documentation on how many threads it
      // will create or run by default.
      this->NewThreadPool = ::CreateThreadpool(nullptr);
      if(unlikely(this->NewThreadPool == nullptr)) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not create thread pool (using Vista and later API)", lastErrorCode
        );
      }

      // Configure the thread pool
      {
        auto closeThreadPoolScope = ON_SCOPE_EXIT_TRANSACTION{
          ::CloseThreadpool(this->NewThreadPool);
        };

        // Set the minimum and maximum number of threads the thread pool can use.
        // Without doing this, we have no idea how many threads the thread pool would use.
        ::SetThreadpoolThreadMaximum(
          this->NewThreadPool, static_cast<DWORD>(maximumThreadCount)
        );
        // This method can't fail, apparently

        BOOL result = ::SetThreadpoolThreadMinimum(
          this->NewThreadPool, static_cast<DWORD>(minimumThreadCount)
        );
        if(unlikely(result == FALSE)) {
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

        closeThreadPoolScope.Commit(); // Everything worked out, don't close the thread pool
      }
    } // if new thread pool API used

    // Keep this mutex locked (it is only opened after a successful shutdown)
    this->ShutdownMutex.lock();
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::~PlatformDependentImplementation() {

    // Destroy all recyclable task so they're gone before the thread pool
    // itself is shut down (unsure if necessary, but definitely cleaner this
    // way since the tasks keep a pointer to the thread pool)
    this->SubmittedTaskPool.DeleteAllRecyclableTasks();

    // Now the thread pool can be safely shut down
    if(this->UseNewThreadPoolApi) {
      ::CloseThreadpool(this->NewThreadPool);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  DWORD WINAPI ThreadPool::PlatformDependentImplementation::oldThreadPoolWorkCallback(
    void *parameter
  ) {
    SubmittedTask *submittedTask = reinterpret_cast<SubmittedTask *>(parameter);

    ON_SCOPE_EXIT {
      std::size_t scheduledTaskCount = (
        submittedTask->Implementation->ScheduledTaskCount.fetch_sub(
          1, std::memory_order::memory_order_release
        )
      );
      if(scheduledTaskCount == 1) { // 1 because we're getting the previous value
        submittedTask->Implementation->ShutdownMutex.unlock();
      }
    };

    bool isShuttingDown = submittedTask->Implementation->IsShuttingDown.load(
      std::memory_order::memory_order_consume // if() below carries dependency
    );
    if(unlikely(isShuttingDown)) {
      submittedTask->Task->~Task();
      submittedTask->Implementation->SubmittedTaskPool.DeleteTask(submittedTask);
    } else {
      ON_SCOPE_EXIT {
        submittedTask->Task->~Task();
        submittedTask->Implementation->SubmittedTaskPool.ReturnTask(submittedTask);
      };
      submittedTask->Task->operator()();
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  void NTAPI ThreadPool::PlatformDependentImplementation::newThreadPoolWorkCallback(
    ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
  ) {
    (void)instance;
    (void)workItem;
    oldThreadPoolWorkCallback(context);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMinimumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMinimumThreadCount(
      countLogicalProcessors()
    );
  }        

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMaximumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMaximumThreadCount(
      countLogicalProcessors()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool(
    std::size_t minimumThreadCount /* = GetDefaultMinimumThreadCount() */,
    std::size_t maximumThreadCount /* = GetDefaultMaximumThreadCount() */
  ) :
    implementation(
      new PlatformDependentImplementation(minimumThreadCount, maximumThreadCount)
    ) {}

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    this->implementation->IsShuttingDown.store(
      true, std::memory_order::memory_order_release
    );

    // The threads have been signalled to shut down, wait until the thread pool
    // has touched all scheduled tasks (they immediately fail and destroy)
    std::size_t remainingTaskCount = (
      this->implementation->ScheduledTaskCount.load(
        std::memory_order::memory_order_consume // if() below carries dependency
      )
    );
    if(remainingTaskCount > 0) {
      bool wasLocked = this->implementation->ShutdownMutex.try_lock_for(
        std::chrono::seconds(5)
      );
      NUCLEX_SUPPORT_NDEBUG_UNUSED(wasLocked);
      assert((wasLocked) && u8"All thread pool tasks were flushed during shutdown");
    }

#if !defined(NDEBUG)
    remainingTaskCount = this->implementation->ScheduledTaskCount.load(
      std::memory_order::memory_order_consume // if() below carries dependency
    );
    assert((remainingTaskCount == 0) && u8"No thread pool tasks remain during shutdown");
#endif

    //this->implementation->SubmittedTaskPook.DeleteAllRecyclableTasks();

    delete this->implementation;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t *ThreadPool::getOrCreateTaskMemory(std::size_t payload) {
    PlatformDependentImplementation::SubmittedTask *submittedTask = (
      this->implementation->SubmittedTaskPool.GetNewTask(payload)
    );
    if(unlikely(submittedTask->Implementation == nullptr)) {
      submittedTask->Implementation = this->implementation;
      if(this->implementation->UseNewThreadPoolApi) {
        submittedTask->Work = ::CreateThreadpoolWork(
          &PlatformDependentImplementation::newThreadPoolWorkCallback,
          nullptr,
          &this->implementation->NewCallbackEnvironment
        );
        if(unlikely(submittedTask->Work == nullptr)) {
          DWORD lastErrorCode = ::GetLastError();
          this->implementation->SubmittedTaskPool.DeleteTask(submittedTask);
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not create thread pool work item", lastErrorCode
          );
        }
      }
    }

    std::uint8_t *submittedTaskMemory = reinterpret_cast<std::uint8_t *>(submittedTask);
    return (
      submittedTaskMemory + offsetof(PlatformDependentImplementation::SubmittedTask, Payload)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::submitTask(std::uint8_t *taskMemory, Task *task) {
    std::uint8_t *submittedTaskMemory = (
      taskMemory - offsetof(PlatformDependentImplementation::SubmittedTask, Payload)
    );
    PlatformDependentImplementation::SubmittedTask *submittedTask = (
      reinterpret_cast<PlatformDependentImplementation::SubmittedTask *>(
        submittedTaskMemory
      )
    );

    submittedTask->Task = task;

    // Increment before executing so we don't risk the task finishing before the increment,
    // dropping the counter lower than 0.
    submittedTask->Implementation->ShutdownMutex.try_lock();
    submittedTask->Implementation->ScheduledTaskCount.fetch_add(
      1, std::memory_order::memory_order_seq_cst
    );

    if(this->implementation->UseNewThreadPoolApi) {
      ::SubmitThreadpoolWork(submittedTask->Work);
    } else {
      ::QueueUserWorkItem(
        &PlatformDependentImplementation::oldThreadPoolWorkCallback,
        submittedTask,
        WT_EXECUTEDEFAULT
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)
