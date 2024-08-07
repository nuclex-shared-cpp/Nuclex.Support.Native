#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOL_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOL_H

#include "Nuclex/Support/Config.h"

// Currently, the thread pool only has implementations for Linux and Windows
//
// The Linux version may be fully or nearly Posix-compatible, so feel free to
// remove this check and give it a try if your system is Posix but not Linux...
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)

#include <cstddef> // for std::size_t
#include <future> // for std::packaged_task, std::future
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
  ///     Optimally, only use the thread pool if you have real number crunching that can be
  ///     parallelized to as many CPU cores as the system can provide. Performing a single task
  ///     in the background or doing something time consuming (like disk accesses) should
  ///     be done with std::async or std::thread instead.
  ///   </para>
  ///   <para>
  ///     Ideally, your tasks would be split into a large number of packages that can each run
  ///     in just a few milliseconds, allowing them to be distributed over many cores and only
  ///     encounter a small period of reduced concurrency at the end when tasks run out.
  ///   </para>
  ///   <para>
  ///     You should not use this thread pool for general purpose tasks or waiting on mutexes,
  ///     at least not with the default thread limits from its default constructor. It would
  ///     quickly clog the thread pool's available threads and render it unable to complete any
  ///     work because just a handful of waiting tasks would fully occupy all the threads.
  ///   </para>
  ///   <para>
  ///     However, it is possible to specify an arbitrarily high maximum thread count and use
  ///     this thread pool for general-purpose work, including long idle waits. Threads will be
  ///     created as needed. In such cases, the use case mentioned earlier (with a large number
  ///     of small work packages) becomes a problem, however, because the thread pool would
  ///     create a silly number of threads and try to run everything at once.
  ///   </para>
  ///   <para>
  ///     In summary, this thread pool has the same caveats as any other thread pool
  ///     implementation. It merely uses defaults that are suitable for number churning rather
  ///     than as a general purpose thread supermarket. In short: know what you're doing :)
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ThreadPool {

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

    // ----------------------------------------------------------------------------------------- //

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

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Schedules a task to be executed on a worker thread</summary>
    /// <typeparam name="TMethod">
    ///   Type of the method that will be run on a worker thread
    /// </typeparam>
    /// <typeparam name="TArguments">
    ///   Type of the arguments that will be passed to the method when it is called
    /// </typeparam>
    /// <param name="method">Method that will be called from a worker thread</param>
    /// <param name="arguments">Argument values that will be passed to the method</param>
    /// <returns>
    ///   An std::future instance that will provide the result returned by the method
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method is your main interface to schedule work on threads of the thread
    ///     pool. Despite the slightly template-heavy signature, it is lean and convenient
    ///     to use. Here's an example:
    ///   </para>
    ///   <example>
    ///     <code>
    ///       int test(int a, int b) {
    ///         Thread::Sleep(milliseconds(10));
    ///         return (a * b) - (a + b);
    ///       }
    ///
    ///       int main() {
    ///         ThreadPool myThreadPool;
    ///
    ///         std::future<int> futureResult = myThreadPool.Schedule<&test>(12, 34);
    ///         int result = futureResult.get(); // waits until result is available
    ///       }
    ///     </code>
    ///   </example>
    ///   <para>
    ///     The returned std::future behaves in every way like an std::future used with
    ///     std::async(). You can ignore it (if your task has no return value), wait
    ///     for a result with std::future::wait() or check its status.
    ///   </para>
    ///   <para>
    ///     Don't be shy about ignoring the returned std::future, the task will still
    ///     run and all std::future handling is inside this header, so the compiler has
    ///     every opportunity to optimize it away.
    ///   </para>
    ///   <para>
    ///     If the thread pool is destroyed before starting on a task, the task will be
    ///     canceled. If you did take hold of the std::future instance, that means it
    ///     will throw an std::future_error of type broken_promise in std::future::get().
    ///   </para>
    /// </remarks>
    public: template<typename TMethod, typename... TArguments>
    inline std::future<typename std::invoke_result<TMethod, TArguments...>::type>
    Schedule(TMethod &&method, TArguments &&... arguments);

    // ----------------------------------------------------------------------------------------- //

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

    /// <summary>Structure to hold platform dependent thread and sync objects</summary>
    private: struct PlatformDependentImplementation;
    /// <summary>Platform dependent thread and sync objects used for the pool</summary>
    private: PlatformDependentImplementation *implementation;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TMethod, typename... TArguments>
  inline std::future<typename std::invoke_result<TMethod, TArguments...>::type>
  ThreadPool::Schedule(TMethod &&method, TArguments &&... arguments) {
    typedef typename std::invoke_result<TMethod, TArguments...>::type ResultType;
    typedef std::packaged_task<ResultType()> TaskType;

    #pragma region struct PackagedTask

    /// <summary>Custom packaged task that carries the method and parameters</summary>
    struct PackagedTask : public Task {

      /// <summary>Initializes the packaged task</summary>
      /// <param name="method">Method that should be called back by the thread pool</param>
      /// <param name="arguments">Arguments to save until the invocation</param>
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
      }

      /// <summary>Stored method pointer and arguments that will be called back</summary>
      public: TaskType Callback;

    };

    #pragma endregion // struct PackagedTask

    // Construct a new task with a callback to the caller-specified method and
    // saved arguments that can subsequently be scheduled on the thread pool.
    std::uint8_t *taskMemory = getOrCreateTaskMemory(sizeof(PackagedTask));
    PackagedTask *packagedTask = new(taskMemory) PackagedTask(
      std::forward<TMethod>(method), std::forward<TArguments>(arguments)...
    );

    // Grab the result before scheduling the task. If the stars are aligned and
    // the thread pool is churning, it may otherwise happen that the task is
    // completed and destroyed between submitTask() and the call to get_future()
    std::future<ResultType> result = packagedTask->Callback.get_future();

    // Schedule for execution. The task will either be executed (default) or
    // destroyed if the thread pool shuts down, both outcomes will result in
    // the future completing with either a result or in an error state.
    submitTask(taskMemory, packagedTask);

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOL_H
