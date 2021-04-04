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

#include <cassert> // for assert()
#include <cmath> // for std::sqrt()

#include "../Helpers/WindowsApi.h"
#include <VersionHelpers.h> // for ::IsWindowsVistaOrGreater()
#include <concurrent_queue.h> // I'd prefer my MPMC queue, but the VS2019 compiler bug...

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
#if 0
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
#endif
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementation {

    #pragma region struct SubmittedTask

    public: struct SubmittedTask {

      /// <summary>
      ///   The instance of the PlatformDependentimplementation class that owns this task
      /// </summary>
      public: PlatformDependentImplementation *Implementation;
      /// <summary>The thread pool work item, if the new thread pool API is used</summary>
      public: ::TP_WORK *Work;
      /// <summary>Side of the payload allocated for this task instance</summary>
      public: std::size_t PayloadSize;
      // <summary>The task instance living in the payload</summary>
      public: ThreadPool::Task *Task;
      /// <summary>This contains a ThreadPool::Task (actually a derived type)</summary>
      public: std::uint8_t Payload[sizeof(std::intptr_t)];

    };

    #pragma endregion // SubmittedTask

    /// <summary>Size of a basic submitted task without its payload</summary>
    public: static const constexpr std::size_t SubmittedTaskFootprint = (
      sizeof(SubmittedTask) - sizeof(std::intptr_t)
    );

    /// <summary>Maximum size of a submitted task to be re-used via the pool</summary>
    /// <remarks>
    ///   If the user submits gigantic tasks, this would otherwise result in filling
    ///   our pool with submitted task instance of gigantic sizes.
    /// </remarks>
    public: static const constexpr std::size_t SubmittedTaskReuseLimit = 256;

    /// <summary>Initializes a platform dependent data members of the process</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    public: PlatformDependentImplementation(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );
    /// <summary>Shuts down the thread pool and frees all resources it owns</summary>
    public: ~PlatformDependentImplementation();

    /// <summary>Creates a new submitted task instance with the requested size</summary>
    /// <param name="payloadSize">Size of the payload to carry in the task</param>
    /// <returns>The submitted task instance with extra memory appended</returns>
    public: SubmittedTask *CreateSubmittedTask(std::size_t payloadSize);

    /// <summary>Destroys a submitted task instance and frees all its resources</summary>
    /// <param name="submittedTask">Submitted task that will be destroyed</param>
    public: void DestroySubmittedTask(SubmittedTask *submittedTask);

    /// <summary>Called by the thread pool to execute a work item</summary>
    /// <param name="context">Task the user has queued for execution</param>
    private: static void NTAPI newThreadPoolWorkCallback(
      ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
    );

    /// <summary>Number of logical processors in the system</summary>
    public: std::size_t ProcessorCount; 
    /// <summary>Whether the thread pool should use the Vista-and-later API</summary>
    public: bool UseNewThreadPoolApi;
    /// <summary>Describes this application (WinSDK version etc.) to the thread pool</summary>
    public: ::TP_CALLBACK_ENVIRON NewCallbackEnvironment;
    /// <summary>Thread pool on which tasks get scheduled if new TP api is used</summary>
    public: ::TP_POOL *NewThreadPool;
    /// <summary>Submitted tasks for re-use</summary>
    public: concurrency::concurrent_queue<SubmittedTask *> SubmittedTaskPool;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::PlatformDependentImplementation(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) :
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
        ::SetThreadpoolThreadMaximum(
          this->NewThreadPool, static_cast<DWORD>(maximumThreadCount)
        );
        // This method can't fail, apparently

        BOOL result = ::SetThreadpoolThreadMinimum(
          this->NewThreadPool, static_cast<DWORD>(minimumThreadCount)
        );
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

        closeThreadPoolScope.Commit(); // Everything worked out, don't close the thread pool
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::~PlatformDependentImplementation() {
    if(this->UseNewThreadPoolApi) {
      ::CloseThreadpool(this->NewThreadPool);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::SubmittedTask *
  ThreadPool::PlatformDependentImplementation::CreateSubmittedTask(
    std::size_t payloadSize
  ) {
    const std::size_t requiredMemory = (
      PlatformDependentImplementation::SubmittedTaskFootprint + payloadSize
    );

    // Allocate enough memory for the submitted task structure and the payload
    // that the caller requested, intended for the Task-derived class instance.
    std::uint8_t *buffer = new std::uint8_t[requiredMemory];
    {
      auto deleteBufferScope = ON_SCOPE_EXIT_TRANSACTION{
        delete[] buffer;
      };

      // Fill the fields we know of (this makes it easier on the bookkeeping side
      // as well... if a submitted tasks exists, its TP_WORK pointer is valid, too).
      {
        SubmittedTask *submittedTask = reinterpret_cast<SubmittedTask *>(buffer);
        submittedTask->Implementation = this;
        submittedTask->PayloadSize = payloadSize;
        submittedTask->Work = ::CreateThreadpoolWork(
          &newThreadPoolWorkCallback, submittedTask, &this->NewCallbackEnvironment
        );
        if(submittedTask->Work == nullptr) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not create thread pool work item", lastErrorCode
          );
        }

        deleteBufferScope.Commit();
        return submittedTask;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::DestroySubmittedTask(
    ThreadPool::PlatformDependentImplementation::SubmittedTask *submittedTask
  ) {
    ::CloseThreadpoolWork(submittedTask->Work);

    // The task is not constructed by the counterpart method, thus neither
    // will this method call its destructor.
    
    delete[] reinterpret_cast<std::uint8_t *>(submittedTask);
  }

  // ------------------------------------------------------------------------------------------- //

  void NTAPI ThreadPool::PlatformDependentImplementation::newThreadPoolWorkCallback(
    ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
  ) {
    SubmittedTask *submittedTask = reinterpret_cast<SubmittedTask *>(context);

    submittedTask->Task->operator()();
    submittedTask->Task->~Task();
    
    submittedTask->Implementation->SubmittedTaskPool.push(submittedTask);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMinimumThreadCount() {
    std::size_t processorCountSquareRoot = static_cast<std::size_t>(
      std::sqrt(countLogicalProcessors()) + 0.5
    );
    if(processorCountSquareRoot >= 4) {
      return processorCountSquareRoot;
    } else {
      return 4;
    }
  }        

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMaximumThreadCount() {
    return countLogicalProcessors() * 2;
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
    delete this->implementation;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t *ThreadPool::getOrCreateTaskMemory(std::size_t payload) {
    std::size_t requiredMemory = (
      PlatformDependentImplementation::SubmittedTaskFootprint + payload
    );

    // If the submitted task would fit into the pool, check the pool for a task
    // that we can re-use instead of allocating extra memory. In the typical, optimal
    // case this will be a single pop() without any fuzz.
    if(likely(requiredMemory < PlatformDependentImplementation::SubmittedTaskReuseLimit)) {
      PlatformDependentImplementation::SubmittedTask *submittedTask;
      std::size_t attempts = 3;
      while(this->implementation->SubmittedTaskPool.try_pop(submittedTask)) {
        if(submittedTask->PayloadSize >= payload) {
          return reinterpret_cast<std::uint8_t *>(submittedTask);
        }

        // We could return it to the pool, but we want task sizes to amortize on
        // the typical payloads of our user, so get rid of this task.
        this->implementation->DestroySubmittedTask(submittedTask);

        --attempts;
        if(attempts == 0) {
          break;
        }
      }
    }

    return (
      reinterpret_cast<std::uint8_t *>(this->implementation->CreateSubmittedTask(payload)) +
      PlatformDependentImplementation::SubmittedTaskFootprint
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::submitTask(Task *task) {
    PlatformDependentImplementation::SubmittedTask *submittedTask = (
      reinterpret_cast<PlatformDependentImplementation::SubmittedTask *>(
        reinterpret_cast<std::uint8_t *>(task) - 
        PlatformDependentImplementation::SubmittedTaskFootprint
      )
    );

    submittedTask->Task = task;

    ::SubmitThreadpoolWork(submittedTask->Work);
  }

  // ------------------------------------------------------------------------------------------- //

  //void ThreadPool::returnTaskMemory(std::uint8_t *taskMemory) {
  //}

/*
  std::size_t ThreadPool::CountMaximumParallelTasks() const {
    return this->implementation->ProcessorCount;
  }
*/
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
