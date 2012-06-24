#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2012 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_THREADING_WINRTTHREADPOOL_H
#define NUCLEX_SUPPORT_THREADING_WINRTTHREADPOOL_H

#include "../Config.h"

#if defined(NUCLEX_SUPPORT_WINRT)

#include "ThreadPool.h"

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Uses the WinRT ThreadPool to distribute work over many threads</summary>
  class WinRTThreadPool : public ThreadPool {

    /// <summary>Initializes a new windows thread pool</summary>
    public: NUCLEX_SUPPORT_API WinRTThreadPool();

    /// <summary>Stops all threads and frees all resources used</summary>
    public: NUCLEX_SUPPORT_API virtual ~WinRTThreadPool();

    /// <summary>Returns the maximum number of tasks that can run in parallel</summary>
    /// <returns>The maximum number of tasks that can be executed in parallel</returns>
    public: NUCLEX_SUPPORT_API std::size_t CountMaximumParallelTasks() const;

    /// <summary>Enqueues a task in the thread pool</summary>
    /// <param name="task">Task that will be enqueued</param>
    /// <param name="count">Times the task will be executed</param>
    public: NUCLEX_SUPPORT_API void AddTask(
      const std::function<void()> &task, std::size_t count = 1
    );

    private: WinRTThreadPool(const WinRTThreadPool &);
    private: WinRTThreadPool &operator =(const WinRTThreadPool &);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WINRT)

#endif // NUCLEX_SUPPORT_THREADING_WINRTTHREADPOOL_H
