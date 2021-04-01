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

#include <exception> // for std::runtime_error
#include <stdexcept> // for std::runtime_error (should be in <exception> but MSVC is weird)
#include <cassert> // for assert()

#include "../Helpers/WindowsApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if the system is running at least the specified Windows version<summary>
  /// <param name="major">Major version number for which to check</param>
  /// <param name="minor">Minor version number for which to check</param>
  /// <returns>True if the system's version numebr is at least the specified version</returns>
  bool isAtLeastWindowsVersion(int major, int minor) {
    ::OSVERSIONINFOW osVersionInfo = { 0 };
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

    // Microsoft declared GetVersionExW() as deprecated, yet relying on the supposed
    // replacement would pin us to Windows 10 or later.
#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4996) // method was declared deprecated
#endif
    BOOL result = ::GetVersionExW(&osVersionInfo);
#ifdef _MSC_VER
  #pragma warning(pop)
#endif
    if(result == FALSE) {
      DWORD errorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not determine operating system version", errorCode
      );
    }

    if(osVersionInfo.dwMajorVersion == static_cast<DWORD>(major)) {
      return (osVersionInfo.dwMinorVersion >= static_cast<DWORD>(minor));
    } else {
      return (osVersionInfo.dwMajorVersion > static_cast<DWORD>(major));
    }
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

  ThreadPool::ThreadPool(std::size_t maximumThreadCount /* = 0 */) {}
    //useNewThreadPoolApi(isAtLeastWindowsVersion(6, 0)) {}

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {}

  // ------------------------------------------------------------------------------------------- //
#if 0
  std::size_t WindowsThreadPool::CountMaximumParallelTasks() const {
    static SYSTEM_INFO systemInfo = SYSTEM_INFO();

    if(systemInfo.dwNumberOfProcessors == 0) {
      ::GetSystemInfo(&systemInfo);
    }

    return static_cast<std::size_t>(systemInfo.dwNumberOfProcessors);
  }

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
