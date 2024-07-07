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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/ConcurrentJob.h"

#if defined(NUCLEX_SUPPORT_WINDOWS) || defined(NUCLEX_SUPPORT_LINUX)

#include "Nuclex/Support/Threading/StopSource.h"
#include "Nuclex/Support/Threading/ThreadPool.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Statuses a concurrent job can be in</summary>
  enum class Status : int {

    /// <summary>Concurrent job is stopped</summary>
    Stopped = 0,
    /// <summary>Concurrent job encountered an error and stopped</summary>
    Failed = 1,
    /// <summary>Concurrent job ran and completed without encountering an error</summary>
    Succeeded = 2,

    /// <summary>Concurrent job is waiting to run</summary>
    Scheduled = -1,
    /// <summary>Concurrent job is currently executing</summary>
    Running = -2,
    /// <summayr>Concurrent job is executing and was signaled to cancel</summary>
    Canceling = -3,
    /// <summayr>Concurrent job is executing, canceled and should restart immediately</summary>
    CancelingWithRestart = -4

  };

  // ------------------------------------------------------------------------------------------- //
  
  /// <summary>Manages calling the DoWork() method of a concurrent job</summary>
  /// <param name="self">This pointer of the concurrent job instance</param>
  /// <param name="doWorkMethod">Pointer to the DoWork() method</param>
  /// <param name="status">Atomic integer maintaining the concurrent job's status</param>
  /// <param name="stateMutex">Mutex that must be held when changing the job's state</param>
  /// <param name="stopTrigger">Souce of stop tokens, changes after each cancel</param>
  /// <param name="statusChangedCondition">
  ///   Condition variable by which users of the concurrent job can wait for completion
  /// </param>
  /// <param name="error">Records any error that happens in the worker thread</param>
  /// <remarks>
  ///   <para>
  ///     This could as well be a private method, possible with a private static method to
  ///     call it, but I wanted to keep this part of the implementation out of the public
  ///     header. There's a lot going on here, including re-running jobs and handling precise
  ///     state transitions the way the concurrent job class expects them.
  ///   </para>
  ///   <para>
  ///     This kind of implementation is pretty efficient, but it's only feasible for code
  ///     like this, which provides a set-in-stone helper class that doesn't need to keep
  ///     evolving and updating its design.
  ///   </para>
  /// </remarks>
  void callDoWorkOnConcurrentJob(
    Nuclex::Support::Threading::ConcurrentJob *self,
    void (Nuclex::Support::Threading::ConcurrentJob::*doWorkMethod)(
      const std::shared_ptr<const Nuclex::Support::Threading::StopToken> &stopToken
    ),
    std::atomic<int> *status,
    std::mutex *stateMutex,
    std::shared_ptr<Nuclex::Support::Threading::StopSource> *stopTrigger,
    std::condition_variable *statusChangedCondition,
    std::exception_ptr *error
  ) {

    // Update the job's state to 'Running' and pick up the currently valid
    // stop token (it will be replaced by a new one if cancellation was used).
    std::shared_ptr<const Nuclex::Support::Threading::StopToken> stopToken;
    {
      bool notifyAllAndReturn = false;
      {
        std::unique_lock<std::mutex> stateMutexScope(*stateMutex);

        int currentStatus = status->load(std::memory_order::memory_order_consume);
        if(currentStatus == static_cast<int>(Status::Canceling)) {
          status->store(
            static_cast<int>(Status::Stopped),
            std::memory_order::memory_order_release
          );
          notifyAllAndReturn = true;
        } else {
          status->store(
            static_cast<int>(Status::Running),
            std::memory_order::memory_order_release
          );

          stopToken = stopTrigger->get()->GetToken();
        }
      } // mutex lock

      if(notifyAllAndReturn) {
        statusChangedCondition->notify_all();
        return;
      }
    } // scope for notifyAllAndReturn

    // We run the status wrangling and DoWork() invocation in a loop because it may be
    // restarted any number of times and we handle this here to reduce the complexity
    // (and chance for mistakes) of the user's overriden DoWork() implementation.
    for(;;) {

      // Invoke the DoWork() method to let the derived class do its background work.
      std::exception_ptr currentError;
      try {
        (self->*doWorkMethod)(stopToken);
      }
      catch(const std::exception &) {
        currentError = std::current_exception();
      }

      // The DoWork() method returned, one way or another. If a restart was pending,
      // we'll just clear the cancel status and run it again right away. Otherwise,
      // a transition to either the succeeded or failed state is needed.
      {
        std::unique_lock<std::mutex> stateMutexScope(*stateMutex);

        int currentStatus = status->load(std::memory_order::memory_order_consume);
        if(currentStatus == static_cast<int>(Status::CancelingWithRestart)) {
          status->store(
            static_cast<int>(Status::Running),
            std::memory_order::memory_order_release
          );

          // If the status was 'Canceling' or 'CancelingWithRestart', a new stop source
          // will have been created to provide a fresh, uncanceled stop token
          stopToken = stopTrigger->get()->GetToken();
          continue;
        }

        // The DoWork() method exited and no restart was scheduled, so the concurrent job
        // is now done and remains in either the failed or succeeded state.
        if(static_cast<bool>(currentError)) {
          *error = currentError;
          status->store(
            static_cast<int>(Status::Failed),
            std::memory_order::memory_order_release
          );
        } else {
          status->store(
            static_cast<int>(Status::Succeeded),
            std::memory_order::memory_order_release
          );
        }

        break;
      } // mutex lock

    } // for(;;)

    statusChangedCondition->notify_all();
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  ConcurrentJob::ConcurrentJob() :
    backgroundThread(),
    threadPool(nullptr),
    status(static_cast<int>(Status::Stopped)),
    stateMutex(),
    stopTrigger(), // leave empty until needed
    statusChangedCondition(),
    error() {}

  // ------------------------------------------------------------------------------------------- //

  ConcurrentJob::ConcurrentJob(ThreadPool &threadPool) :
    backgroundThread(),
    threadPool(&threadPool),
    status(static_cast<int>(Status::Stopped)),
    stateMutex(),
    stopTrigger(), // leave empty until needed
    statusChangedCondition(),
    error() {}

  // ------------------------------------------------------------------------------------------- //

  ConcurrentJob::~ConcurrentJob() {
    Cancel();

    // Wait until the background thread has finished. We could use Join() here,
    // but we don't want an exception to be re-thrown in the destructor.    
    for(;;) {

      // Mutex lock scope
      {
        std::unique_lock<std::mutex> stateMutexScope(this->stateMutex);

        int currentStatus = this->status.load(std::memory_order::memory_order_consume);
        if(currentStatus >= 0) {
          break;
        }

        this->statusChangedCondition.wait(stateMutexScope);
      }

    } // for(;;)

    // Finally, if the background thread was running, join it to prevent
    // the platform's standard C++ library from calling std::terminate() out of frustration.
    if(this->backgroundThread.joinable()) {
      this->backgroundThread.join();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool ConcurrentJob::IsRunning() const {
    int currentStatus = this->status.load(std::memory_order::memory_order_relaxed);
    return (currentStatus < 0);
  }

  // ------------------------------------------------------------------------------------------- //

  void ConcurrentJob::StartOrRestart() {

    // Figure out if we can order the already running worker to cancel and restart
    // or whether we have to schedule a new execution of the worker.
    bool startNewWorker = false;
    {
      std::unique_lock<std::mutex> stateMutexScope(this->stateMutex);

      int currentStatus = this->status.load(std::memory_order::memory_order_consume);
      if(currentStatus == static_cast<int>(Status::Running)) {
        this->status.store( // Currently running, cancel and repeat DoWork() call
          static_cast<int>(Status::CancelingWithRestart),
          std::memory_order::memory_order_release
        );
        this->stopTrigger->Cancel();
        this->stopTrigger = StopSource::Create();
      } else if(currentStatus == static_cast<int>(Status::Canceling)) {
        this->status.store( // Already canceled, ask to repeat DoWork() call
          static_cast<int>(Status::CancelingWithRestart),
          std::memory_order::memory_order_release
        );
      } else if(currentStatus >= 0) { // If the worker was not running, start a new one
        this->status.store(
          static_cast<int>(Status::Scheduled),
          std::memory_order::memory_order_release
        );
        if(!static_cast<bool>(this->stopTrigger)) {
          this->stopTrigger = StopSource::Create();
        }
        startNewWorker = true;
      }
    } // mutex lock

    // If, at the time we were holding the lock, no worker was running or
    // scheduled to run, start a new one here.
    if(startNewWorker) {
      if(this->threadPool == nullptr) {
        std::thread callDoWorkThread(
          &callDoWorkOnConcurrentJob,
          this,
          &ConcurrentJob::DoWork,
          &this->status,
          &this->stateMutex,
          &this->stopTrigger,
          &this->statusChangedCondition,
          &this->error
        );

        // If any prior thread was being held, it will be destroyed here.
        this->backgroundThread.swap(callDoWorkThread);
        if(callDoWorkThread.joinable()) {
          callDoWorkThread.join();
        }
      } else {
        this->threadPool->Schedule(
          &callDoWorkOnConcurrentJob,
          this,
          &ConcurrentJob::DoWork,
          &this->status,
          &this->stateMutex,
          &this->stopTrigger,
          &this->statusChangedCondition,
          &this->error
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ConcurrentJob::Cancel() {
    std::unique_lock<std::mutex> stateMutexScope(this->stateMutex);

    int currentStatus = this->status.load(std::memory_order::memory_order_consume);
    if(
      (currentStatus == static_cast<int>(Status::Running)) ||
      (currentStatus == static_cast<int>(Status::Scheduled))
    ) {
      this->status.store(
        static_cast<int>(Status::Canceling),
        std::memory_order::memory_order_release
      );
      this->stopTrigger->Cancel();
      this->stopTrigger = StopSource::Create();
    } else if(currentStatus == static_cast<int>(Status::CancelingWithRestart)) {
      this->status.store(
        static_cast<int>(Status::Canceling),
        std::memory_order::memory_order_release
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool ConcurrentJob::Join(
    std::chrono::microseconds patience /* = std::chrono::microseconds() */
  ) {
    std::chrono::time_point end = std::chrono::steady_clock::now() + patience;
    for(;;) {
      
      // Extra scope for mutex so it is dropped and reacquired (needlessly?).
      // The condition_variable::wait_until() will already make the mutex lockable
      // for the background thread, but this feels less spooky to me.
      {
        std::unique_lock<std::mutex> stateMutexScope(this->stateMutex);

        int currentStatus = this->status.load(std::memory_order::memory_order_consume);
        switch(currentStatus) {
          case static_cast<int>(Status::Stopped): {
            return true;
          }
          case static_cast<int>(Status::Succeeded): {
            this->status.store(
              static_cast<int>(Status::Stopped),
              std::memory_order::memory_order_release
            );
            return true;
          }
          case static_cast<int>(Status::Failed): {
            this->status.store(
              static_cast<int>(Status::Stopped),
              std::memory_order::memory_order_release
            );
            std::rethrow_exception(this->error);
          }
        }

        // This will open the mutex for other threads to enter and wait until the condition
        // variable is signaled. The worker thread signals the condition variable whenever
        // a status change occurs that would be intersting to this specific line of code.
        if(patience.count() == 0) {
          this->statusChangedCondition.wait(stateMutexScope);
        } else {
          std::cv_status result = this->statusChangedCondition.wait_until(stateMutexScope, end);
          if(result == std::cv_status::timeout) {
            return false; // If the wait timed out, 
          }
        }
      } // mutex scope

    } // for(;;)

    // Could call thread::join() here, but another thread might have called StartOrRestaty()
    // already and we'd be sitting here until the whole operation is done...
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WINDOWS) || defined(NUCLEX_SUPPORT_LINUX)
