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

#include <cstddef> // for std::size_t
#include <future> // for std::packaged_task
#include <functional> // for std::bind

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Distributes tasks to several threads</summary>
  /// <remarks>
  ///   <para>
  ///     On some platforms (the Microsoft ones), creating a new threads is a heavy operation
  ///     that makes it unsuitable for micro tasks, like parallelizing a mere loop.
  ///   </para>
  ///   <para>
  ///     With the thread pool, a bunch of threads are created up front and simply wait for
  ///     a task. This allows tasks of fine granularity to be split into multiple threads
  ///     without having the setup time exceed the gains.
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
  ///   <para>
  ///     Ideally, your tasks would be split into packages that can be done in a millisecond
  ///     or less, allowing even 
  ///   </para>
  /// </remarks>
  class ThreadPool {

    #pragma region class Task

    /// <summary>Base class for tasks that get executed by the thread pool</summary>
    /// <remarks>
    ///   Only used internally and does some creative memory acrobatics. Don't expose!
    /// </remarks>
    private: class Task {

      /// <summary>Terminates the task. If the task was not executed, cancels it</summary>
      public: virtual ~Task() = default;
      /// <summary>Executes the task. Is called on the thread pool thread</summary>
      public: virtual void operator()() = 0;

    };

    #pragma endregion // class Task

    /// <summary>Determines a good base number of threads to keep active</summary>
    /// <returns>The default minimum number of threads for new thread pools</returns>
    public: NUCLEX_SUPPORT_API static std::size_t GetDefaultMinimumThreadCount();

    /// <summary>Determines a good maximum number of threads for a thread pool</summary>
    /// <returns>The default maximum number of threads for new thread pools</returns>
    public: NUCLEX_SUPPORT_API static std::size_t GetDefaultMaximumThreadCount();

    /// <summary>Initializes a new thread pool</summary>
    /// <param name="minimumThreadCount">
    ///   Number of threads that will be created up-front and always stay active
    /// </param>
    /// <param name="maximumThreadCount">
    ///   Highest number of threads to which the thread pool can grow under load
    /// </param>
    public: NUCLEX_SUPPORT_API ThreadPool(
      std::size_t minimumThreadCount = GetDefaultMinimumThreadCount(),
      std::size_t maximumThreadCount = GetDefaultMaximumThreadCount()
    );

    /// <summary>Stops all threads and frees all resources used</summary>
    public: NUCLEX_SUPPORT_API ~ThreadPool();

    /// <summary>Schedules a task to be executed on a worker thread</summary>
    /// <typeparam name="TMethod">Method that will be run on a worker thread</typeparam>
    public: template<typename TMethod, typename... TArguments>
    std::future<typename std::result_of<TMethod(TArguments...)>::type>
    AddTask(TMethod &&method, TArguments &&... arguments) {
      typedef typename std::result_of<TMethod(TArguments...)>::type ResultType;
      typedef std::packaged_task<ResultType()> TaskType;

      /// <summary>Custom packaged task that carries the method and parameters</summary>
      struct PackagedTask : public Task {

        /// <summary>Initializes the packaged task</summary>
        public: PackagedTask(TMethod &&method, TArguments &&... arguments) :
          Task(),
          Callback(
            std::bind(std::forward<TMethod>(method), std::forward<TArguments>(arguments)...)
          ) {}

        /// <summary>Terminates the task. If the task was not executed, cancels it</summary>
        public: ~PackagedTask() override = default;

        /// <summary>Executes the task. Is called on the thread pool thread</summary>
        public: void operator()() override {
          this->Callback();
          //std::atomic_thread_fence(std::memory_order::memory_order_release);
          //this->wasExecuted.store(true, std::memory_order_relaxed);
        }

        /// <summary>Stored method pointer and arguments that will be called back</summary>
        public: TaskType Callback;
        //private: std::atomic<bool> wasExecuted;
      };

      std::uint8_t *taskMemory = getOrCreateTaskMemory(sizeof(PackagedTask));
      PackagedTask *packagedTask = new(taskMemory) PackagedTask(
        std::forward<TMethod>(method), std::forward<TArguments>(arguments)...
      );
      submitTask(taskMemory, packagedTask);

      return packagedTask->Callback.get_future();
    }

    /// <summary>
    ///   Creates (or fetches from the pool) a task with the specified payload size
    /// </summary>
    /// <param name="payload">Size of the task instance</param>
    /// <returns>A new or reused task with at least the requested payload size</returns>
    private: NUCLEX_SUPPORT_API std::uint8_t *getOrCreateTaskMemory(std::size_t payload);

    /// <summary>
    ///   Submits a task (created via getOrCreateTaskMemory()) to the thread pool
    /// </summary>
    /// <param name="taskMemory">Memory block returned by getOrCreateTaskMemory</param>
    /// <param name="task">Task that will be submitted</param>
    private: NUCLEX_SUPPORT_API void submitTask(std::uint8_t *taskMemory, Task *task);


/* Nope, that will happen automatically when the task finished
    /// <summary>
    ///   Gives a memory block back that was 
    ///
    private: NUCLEX_SUPPORT_API returnTaskMemory(std::uint8_t *taskMemory);
*/
    //private: NUCLEX_SUPPORT_API virtual void SubmitTask()

/*
    /// <summary>Returns the maximum number of tasks that can run in parallel</summary>
    /// <returns>The maximum number of tasks that can be executed in parallel</returns>
    /// <remarks>
    ///   Depending on the implementation, this may be equal to the number of worker
    ///   threads or it may be completely unrelated. It should be equal to
    ///   std::thread::hardware_concurrency() in all recent C++ versions.
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::size_t CountMaximumParallelTasks() const;

    /// <summary>Create a default thread pool for the system</summary>
    /// <returns>The default thread pool on the current system</returns>
    public: NUCLEX_SUPPORT_API static std::shared_ptr<ThreadPool> CreateSystemDefault();


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

    /// <summary>Structure to hold platform dependent thread and sync objects</summary>
    private: struct PlatformDependentImplementation;
    /// <summary>Platform dependent process and file handles used for the process</summary>
    private: PlatformDependentImplementation *implementation;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOL_H
