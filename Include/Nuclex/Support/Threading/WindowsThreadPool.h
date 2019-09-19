#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_THREADING_WINDOWSTHREADPOOL_H
#define NUCLEX_SUPPORT_THREADING_WINDOWSTHREADPOOL_H

#include "../Config.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "ThreadPool.h"

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Uses the Win32 ThreadPool API to distribute work over many threads</summary>
  class WindowsThreadPool : public ThreadPool {

    /// <summary>Initializes a new Windows thread pool</summary>
    public: NUCLEX_SUPPORT_API WindowsThreadPool();

    /// <summary>Stops all threads and frees all resources used</summary>
    public: NUCLEX_SUPPORT_API virtual ~WindowsThreadPool();

    /// <summary>Returns the maximum number of tasks that can run in parallel</summary>
    /// <returns>The maximum number of tasks that can be executed in parallel</returns>
    public: NUCLEX_SUPPORT_API std::size_t CountMaximumParallelTasks() const;

    /// <summary>Enqueues a task in the thread pool</summary>
    /// <param name="task">Task that will be enqueued</param>
    /// <param name="count">Times the task will be executed</param>
    public: NUCLEX_SUPPORT_API void AddTask(
      const std::function<void()> &task, std::size_t count = 1
    );

    /// <summary>Determines if at least the specified Windows version is running</summary>
    /// <param name="major">Major version number, eg. 6 for Windows Vista</param>
    /// <param name="minor">Minor version number, eg. 1 for Windows 7</param>
    /// <returns>True if the user is running at least that Windows version</returns>
    private: static bool isAtLeastWindowsVersion(int major, int minor);

    private: WindowsThreadPool(const WindowsThreadPool &);
    private: WindowsThreadPool &operator =(const WindowsThreadPool &);

    /// <summary>Whether the new thread pool API introduced with Vista will be used</summary>
    private: bool useNewThreadPoolApi;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_THREADING_WINDOWSTHREADPOOL_H
