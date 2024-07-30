Base class for Concurrent Jobs
==============================

This is a base class that lets you run some code in a background thread.
While you can achieve all of this using `std::thread` or
`Nuclex::Threading::ThreadPool` directly, after finding myself repeatedly
implementing such classes, including the same code to re-throw exceptions,
forwarding stop tokens and all, I generalized them all into this base class.

It offers the following features:

- Run code in a background thread
- Catch exceptions and have them resurface from `Join()`
- Support cancelation via stop tokens
- Allow multiple executions
- Restartable, meaning the current job cancels and restarts


Usage
-----

In the simplest case, just inherit your own class from
`Nuclex::Support::Threading::ConcurrentJob`:

```cpp
class LiveDataUpdater : public Nuclex::Support::Threading::ConcurrentJob {

  /// <summary>Called in the background thread to perform the actual work</summary>
  /// <param name="canceler">Token by which the operation can be signalled to cancel</param>
  protected: void DoWork(
    const std::shared_ptr<const Nuclex::Support::Threading::StopToken> &canceler
  ) override;

};
```

The `DoWork()` method will be invoked in the background thread. The stop token should be checked
in regular intervals to detect cancelation and will trigger when the job is manually cancelled,
when it is restarted and in the destructor, too (*though you should absolutely override
the destructor and cancel yourself in the derived class, otherwise the resources in your derived
class will already have been destroyed while the thread was still running*).

Now there are a few methods that control the background thread's execution:

```cpp
class ConcurrentJob {

  // ...

  /// <summary>Starts the background job</summary>
  /// <remarks>
  ///   This will start the background job unless it is already running, in which
  ///   case it will do nothing.
  /// </remarks>
  protected: void Start();

  /// <summary>Starts or restarts the background job</summary>
  /// <remarks>
  ///   If the background job was already running, this cancels it, then lifts
  ///   the cancellation and starts over. If another thread is blocking on
  ///   <see cref="Join" />, it will continue to block until the background job
  ///   ends without having a restart scheduled.
  /// </remarks>
  protected: void StartOrRestart();

  /// <summary>Cancels the background job</summary>
  protected: void Cancel();

  /// <summary>Waits for the thread to exit</summary>
  /// <param name="patience">Maximum amount of time to wait for the job to finish</param>
  /// <returns>True if the job finished, false if the patience time was exceeded</returns>
  /// <remarks>
  ///   This method will only wait but not check for errors or do anything else. You
  ///   can use it to wait for the background thread to finish in your destructor or
  ///   if you're using alternative error handling methods.
  /// </remarks>
  protected: bool Wait(
    std::chrono::microseconds patience = std::chrono::microseconds()
  );

  /// <summary>
  ///   Waits for the thread to exit and re-throws any exception that occurred
  /// </summary>
  /// <param name="patience">Maximum amount of time to wait for the job to finish</param>
  /// <returns>True if the job finished, false if the patience time was exceeded</returns>
  /// <remarks>
  ///   This method should only be called by one thread. If an exception happened inside
  ///   the thread doing the work in the background, it will be re-thrown from this
  ///   method. It is fine to not call Join() at all. Multiple calls are okay, too.
  /// </remarks>
  protected: bool Join(
    std::chrono::microseconds patience = std::chrono::microseconds()
  );

  // ...
```

These are `protected` because it is up to you how you wish your specific
background thread to be controlled.


Outside Control over Start/Stop
-------------------------------

One approach is to allow your concurrent job to be started and stopped from
the outside. This can be useful if it's just a simple one-time job, like
saving a file while the UI thread displays a progress window.

The controlling party usually also wants to receive any exceptions that
happened in the background thread so it can report any failures to the user.

In such a case, you can simply expose those three methods to the outside:

```cpp
class DocumentExportJob : public Nuclex::Support::Threading::ConcurrentJob {

  // ...

  public: using Nuclex::Support::Threading::ConcurrentJob::Start();
  public: using Nuclex::Support::Threading::ConcurrentJob::Cancel();
  public: using Nuclex::Support::Threading::ConcurrentJob::Join();

  // ...


};
```


As a Background Service
-----------------------

Another usage of the concurrent job is when you need to check on the state of
a file or resource at regular intervals. In such cases, you can keep all
control methods protected (only accessible internally) and start or restart
the job whenever its settings change:

```cpp
class ProcessWatchdogJob : public Nuclex::Support::Threading::ConcurrentJob {

  public: override ~ProcessWatchdogJob() {
    Cancel();
    Wait();
  }

  public: void SetPid(::pid_t pid) {
    {
      std::unique_lock<std::mutex> propertyUpdateScope(this->propertyUpdateMutex);
      this->pid = pid;
    }
    StartOrRestart();
  }

  public: void SetCheckingInterval(std::chrono::seconds interval) {
    {
      std::unique_lock<std::mutex> propertyUpdateScope(this->propertyUpdateMutex);
      this->interval = interval;
    }
    StartOrRestart();
  }

  // ...

};
```

A small note on the efficiency of this, since you'll likely have code similar
to the following during your application's startup:

```cpp
this->watchdog = std::make_shared<ProcessWatchdogJob>();
this->watchdog->SetPid(currentPid);
this->watchdog->SetCheckingInterval(std::chrono::seconds(15));
```

There's always a bit of latency involved when launching threads. More if you
run on an `std::thread` and less if you use `Nuclex::Threading::ThreadPool`,
but either way, the thread will not run instantaneously.

The `StartOrRestart()` method makes use of this. If there is a restart already
pending, asking for another restart is a no-op. This means that until
the background thread has both launched and also entered the `DoWork()`
method, additional calls to `StartOrRestart()` will not go through a full
cancelation cycle and the price you pay is merely a mutex lock and flag check.

This is, of course, a form of harmless race condition. If the above code
snippet was delayed between its `SetPid()` and `SetCheckingInterval()` calls,
enough to fully launch the background thread, the background thread will
indeed be canceled and restarted (however, it will just involve exiting
`DoWork()` and calling it again in the same thread with a new stop token).


Using `std::thread` vs thread pooling
-------------------------------------

The decision is the same as everywhere else. If you have a long-running
operation, such as saving a document, printing it, downloading a file or any
other thing that's *not* parallel processing of data chunks, it is more
appropriate to use `std::thread`.

The default constructor of the `ConcurrentJob` class will do just that.

There also is a constructor that accepts a `Nuclex::Threading::ThreadPool`,
using a thread pool thread to run your `DoWork()` method.

For most usages of the `ConcurrentJob`, that is not what you want. But if you
are writing very specialized code, for example, an application that causes
a lot of CPU load but wants to leave one core free for the UI thread at all
tiems, it is a good option to control resource usage.

Another usage where the thread pool may make sense is if you also use
`ConcurrentJob` for smaller background tasks, such as batch processing of
documents, images or shaders. That can be useful if you want to benefit from
the exception isolation, re-throwing and stop token facilities.
