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

#ifndef NUCLEX_SUPPORT_THREADING_CONCURRENTJOB_H
#define NUCLEX_SUPPORT_THREADING_CONCURRENTJOB_H

#include "Nuclex/Support/Config.h"

#include <exception> // for std::exception
#include <thread> // for std::thread
#include <mutex> // for std::mutex
#include <condition_variable> // for std::condition_variable
#include <atomic> // for std::atomic
#include <chrono> // for std::chrono::milliseconds
#include <memory> // for std::shared_ptr

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  class ThreadPool;
  class StopSource;
  class StopToken;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Job running in a background thread that can be run, restarted and canceled
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     This is a repeatable job. You can use it as a base class for things that need to
  ///     happen in the background and even expose it under some interface (or wrap it) in
  ///     order to let callers start, cancel or restart the operation freely.
  ///   </para>
  ///   <para>
  ///     The ConcurrentJob class is designed for higher-level tasks, for example to run
  ///     a printing or exporting job in the background while the UI thread keeps servicing
  ///     the UI. Calling <see cref="StartOrRestart" /> blocks until the thread to actually
  ///     starts executing to ensure the next thing the calling thread will see is a truthful
  ///     <see cref="IsRunning" /> flag set to true and it catches exceptions and re-throws
  ///     them when you join with the background thread.
  ///   </para>
  ///   <para>
  ///     The <see cref="StartOrRestart" />, <see cref="Cancel" /> and <see cref="Join" />
  ///     methods are protected in this class. This is intentional to give you control over
  ///     what methods will be exposed publicly in your inheriting class. If these methods
  ///     are okay for your case to have in your inheriting class' public interface, simply
  ///     write three using statements to expose them:
  ///   <para>
  ///   <example>
  ///     <code>
  ///       class MyBackgroundOperation : public ConcurrentJob {
  ///         // ...
  ///         public: using ConcurrentJob::StartOrRestart();
  ///         public: using ConcurrentJob::Cancel();
  ///         public: using ConcurrentJob::Join();
  ///         // ..
  ///       };
  ///     </code>
  ///   </example>
  ///   <para>
  ///     Though in any case where your background operation has a result it returns, you
  ///     probably want to at least wrap <see cref="Join" /> with a custom return value that
  ///     your <see cref="DoWork" /> override stores upon finishing.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ConcurrentJob {

    /// <summary>Initializes a new concurrent job</summary>
    public: NUCLEX_SUPPORT_API ConcurrentJob();
    /// <summary>Initializes a new concurrent job using a thread pool thread</summary>
    /// <param name="threadPool">Thread pool in which the concurrent job will run</param>
    public: NUCLEX_SUPPORT_API ConcurrentJob(ThreadPool &threadPool);

    /// <summary>Cancels the thread if running and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~ConcurrentJob();

    /// <summary>Whether the background job is current running</summary>
    /// <remarks>
    ///   Don't use this to make decisions, use it to display a progress spinner in your UI
    ///   or somthing similarly inconsequential.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool IsRunning() const;

    /// <summary>Starts or restarts the background job</summary>
    /// <remarks>
    ///   If the background job was already running, this cancels it, then lifts
    ///   the cancellation and starts over. If another thread is blocking on
    ///   <see cref="Join" />, it will continue to block until the background job
    ///   ends without having a restart scheduled.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API void StartOrRestart();

    /// <summary>Cancels the background job</summary>
    protected: NUCLEX_SUPPORT_API void Cancel();

    /// <summary>
    ///   Waits for the thread to exit and re-throws any exception that occurred
    /// </summary>
    /// <param name="patience">Maximum amount of time to wait for the job to finish</param>
    /// <returns>True if the job finished, false if the patience time was exceeded</returns>
    /// <remarks>
    ///   This method should only be called by one thread. If an exception happened inside
    ///   the threading doing the work in the background, it will be re-thrown from this
    ///   method. It is fine to not call Join() at all.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API bool Join(
      std::chrono::microseconds patience = std::chrono::microseconds()
    );

    /// <summary>Called in the background thread to perform the actual work</summary>
    /// <param name="canceler">Token by which the operation can be signalled to cancel</param>
    /// <remarks>
    ///   If the work being performed takes more than a few milliseconds, you should regularly
    ///   check if the job has been cancelled. If the job is cancelled, this method should just
    ///   return. When a restart or another execution is scheduled, the <see cref="DoWork" />
    ///   method will run on the same thread again right away.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual void DoWork(
      const std::shared_ptr<const StopToken> &canceler
    ) = 0;

#if 0 // Could be useful if the inherited class wants to signal something with an event
    /// <summary>Called in the background thread when <see cref="DoWork" /> exits</summary>
    /// <remarks>
    ///   This is fired regardless of whether the <see cref="DoWork" /> method completed
    ///   normally or exited due to an exception. What you likely want to to here is notify
    ///   your UI thread to call <see cref="Join" /> in order to either pick up the result
    ///   or receive the exception, handle it and display an appropriate message to the user.
    /// </remarks>
    protected: virtual void WorkFinished() {}
#endif

    /// <summary>Thread that is running in the background</summary>
    /// <remarks>
    ///   This is used if concurrent job is constructed without a thread pool
    /// </remarks>
    private: std::thread backgroundThread;
    /// <summary>If set, the concurrent job uses the thread pool to run workers</summary>
    private: ThreadPool *threadPool;
    /// <summary>Whether the current thread is still running</summary>
    private: std::atomic<int> status;
    /// <summary>Needs to be be held when changing the state of the thread</summary>
    private: std::mutex stateMutex;
    /// <summary>Used to ask background worker to cancel when needed</summary>
    private: std::shared_ptr<StopSource> stopTrigger;
    /// <summary>Used to wait for the thread to start running</summary>
    /// <remarks>
    ///   The <see cref="StartOrRestart" /> method waits until the thread is actually running
    ///   and has the <see cref="status" /> flag set before returning. That ensures there
    ///   is no confusion about the state if two threads call <see cref="StartOrRestart" />.
    /// </remarks>
    private: std::condition_variable statusChangedCondition;
    /// <summary>Records any exception that has happened in the background thread</summary>
    private: std::exception_ptr error;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_CONCURRENTJOB_H
