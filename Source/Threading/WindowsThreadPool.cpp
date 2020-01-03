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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/WindowsThreadPool.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

namespace {

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
      std::unexpected();
    }

    if(::InterlockedDecrement(&task->second) == 0) {
      delete task;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  WindowsThreadPool::WindowsThreadPool() :
    useNewThreadPoolApi(isAtLeastWindowsVersion(6, 0)) {}

  // ------------------------------------------------------------------------------------------- //

  WindowsThreadPool::~WindowsThreadPool() {}

  // ------------------------------------------------------------------------------------------- //

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

  // ------------------------------------------------------------------------------------------- //

  bool WindowsThreadPool::isAtLeastWindowsVersion(int major, int minor) {
    OSVERSIONINFOW osVersionInfo = {0};
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    BOOL result = ::GetVersionExW(&osVersionInfo);
    if(result == FALSE) {
      throw std::runtime_error("Could not determine operating system version");
    }

    if(osVersionInfo.dwMajorVersion == static_cast<DWORD>(major)) {
      return (osVersionInfo.dwMinorVersion >= static_cast<DWORD>(minor));
    } else {
      return (osVersionInfo.dwMajorVersion > static_cast<DWORD>(major));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)
