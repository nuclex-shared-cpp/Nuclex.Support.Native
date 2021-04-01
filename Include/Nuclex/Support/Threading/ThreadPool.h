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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOL_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOL_H

#include "Nuclex/Support/Config.h"

#include <cstddef>
#include <functional>
#include <memory>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Distributes tasks to several threads</summary>
  /// <remarks>
  ///   <para>
  ///     One some platforms (the Microsoft ones), creating a new threads is a heavy operation
  ///     that makes it unsuitable for micro tasks like parallelizing a mere loop.
  ///   </para>
  ///   <para>
  ///     With the thread pool, at least moderately-sized tasks can be split into multiple
  ///     threads without making the setup time exceed the gains. It uses the platform's
  ///     most efficient method of keeping threads ready (which may even be to delegate to
  ///     an OS-provided thread pool).
  ///   </para>
  ///   <para>
  ///     Do not use the thread pool for general purpose tasks or waiting on mutexes. It would
  ///     immediately prevent the thread pool from performing work for 1 or more CPU cores due
  ///     to the threads being stuck on the wait.
  ///   </para>
  ///   <para>
  ///     Only use the thread pool if you have real number crunching that can be parallelized
  ///     to as many CPU cores as the system can provide. Performing a single task in
  ///     the background or doing something time consuming (like disk accesses) should always
  ///     be done with std::async or std::thread.
  ///   </para>
  /// </remarks>
  class ThreadPool {

    /// <summary>Initializes a new thread pool</summary>
    /// <param name="maximumThreadCount">
    ///   Maximum number of threads the thread pool will spawn. Use 0 to 
    /// </param>
    public: NUCLEX_SUPPORT_API ThreadPool(std::size_t maximumThreadCount = 0);

    /// <summary>Stops all threads and frees all resources used</summary>
    public: NUCLEX_SUPPORT_API virtual ~ThreadPool();

/*
    /// <summary>Create a default thread pool for the system</summary>
    /// <returns>The default thread pool on the current system</returns>
    public: NUCLEX_SUPPORT_API static std::shared_ptr<ThreadPool> CreateSystemDefault();

    /// <summary>Returns the maximum number of tasks that can run in parallel</summary>
    /// <returns>The maximum number of tasks that can be executed in parallel</returns>
    public: NUCLEX_SUPPORT_API virtual std::size_t CountMaximumParallelTasks() const = 0;

    /// <summary>Enqueues a task in the thread pool</summary>
    /// <param name="task">Task that will be enqueued</param>
    /// <param name="count">Times the task will be executed</param>
    /// <remarks>
    ///   <para>
    ///     Tasks will be executed by the next available thread. The more tasks you use
    ///     the more work can potentially be performed in parallel. If you want to
    ///     split a single piece of work so it can be done in parallel, you can partition
    ///     the data into n pieces (where n >= CountMaximumParallelTasks() ideally) and
    ///     use the <see cref="count" /> parameter to specify how often the task should
    ///     be invoked.
    ///   </para>
    ///   <para>
    ///     Tasks should never throw an exception. If any exception gets through,
    ///     std::unexpected will be invoked, which usually means the immediate termination
    ///     of the application. 
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void AddTask(
      const std::function<void()> &task, std::size_t count = 1
    ) = 0;

    private: ThreadPool(const ThreadPool &);
    private: ThreadPool &operator =(const ThreadPool &);
*/
  };


  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOL_H
