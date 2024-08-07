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

#include "Nuclex/Support/Threading/Latch.h"

#if defined(NUCLEX_SUPPORT_LINUX) // Directly use futex via kernel syscalls
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetRemainingTimeout()
#include "../Platform/LinuxFutexApi.h" // for LinuxFutexApi::PrivateFutexWait() and more
#elif defined(NUCLEX_SUPPORT_WINDOWS) // Use standard win32 threading primitives
#include "../Platform/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#include "../Platform/WindowsSyncApi.h" // for ::WaitOnAddress(), ::WakeByAddressAll()
#include <mutex> // for std::mutex
#else // Posix: use a pthreads conditional variable to emulate a semaphore
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <ctime> // for ::clock_gettime()
#include <pthread.h> // for ::pthread_cond_init() etc.
#endif

#include <atomic> // for std::atomic
#include <cassert> // for assert()

#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS)
  // Just some safety checks to make sure pthread_condattr_setclock() is available.
  // https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
  //
  // You shouldn't encounter any Linux system where the Posix implementation isn't set
  // to Posix 2008-09 or something newer by default. If you do, you can set _POSIX_C_SOURCE
  // in your build system or remove the Latch implementation from your library.
  #if defined(_POSIX_C_SOURCE)
    #if (_POSIX_C_SOURCE < 200112L)
      #error Your C runtime library needs to at least implement Posix 2001-12
    #endif
    //#if !defined(__USE_XOPEN2K)
  #endif
#endif

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Latch::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the latch</summary>
    /// <param name="initialCount">Initial admit count for the latch</param>
    public: PlatformDependentImplementationData(std::size_t initialCount);

    /// <summary>Frees all resources owned by the Latch</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_LINUX)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: mutable volatile std::uint32_t FutexWord;
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: mutable volatile std::uint32_t WaitWord;
#else // Posix
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
#endif
    /// <summary>How many tasks the latch is waiting on</summary>
    public: std::atomic<std::size_t> Countdown;

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    FutexWord((initialCount > 0) ? 0 : 1),
    Countdown(initialCount) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    WaitWord((initialCount > 0) ? 0 : 1),
    Countdown(initialCount) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    Condition(),
    Mutex(),
    Countdown(initialCount) {

    // Attribute necessary to use CLOCK_MONOTONIC for condition variable timeouts
    ::pthread_condattr_t *monotonicClockAttribute = (
      Platform::PosixTimeApi::GetMonotonicClockAttribute()
    );

    // Create a new pthread conditional variable
    int result = ::pthread_cond_init(&this->Condition, monotonicClockAttribute);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable", result
      );
    }

    result = ::pthread_mutex_init(&this->Mutex, nullptr);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)
  Latch::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    // Nothing to do. If threads are waiting, they're now waiting on dead memory.
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Latch::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Latch::Latch(std::size_t initialCount) :
    implementationDataBuffer() {

    // If this assert hits, the buffer size assumed by the header was too small.
    // There will be a buffer overflow in the line after and the application will
    // likely crash or at least malfunction.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Latch fits in buffer"
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initialCount);
  }

  // ------------------------------------------------------------------------------------------- //

  Latch::~Latch() {
    getImplementationData().~PlatformDependentImplementationData();
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the latch counter. This locks the latch.
    std::size_t previousCountdown = impl.Countdown.fetch_add(
      count, std::memory_order_release
    );

    // If the latch was open at the time of this call, we need to close it so threads
    // can wait on the futex.
    if(unlikely(previousCountdown == 0)) {

      // There's a race condition at this point. If at this exact point, another thread
      // calls CountDown() and sets the FutexWord to 1 because the counter hit zero,
      // we'd falsely change it back to 0.
      //
      // Then a third thread which saw the countdown being greater than zero might reach
      // the futex wait and actually begin waiting even though the latch should be open.
      //
      // To fix this, we re-check the latch counter after setting the futex word.
      //
      if(likely(count > 0)) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked
      }

      // Re-check the latch counter. This might seem like a naive hack at first sight,
      // but by only doing this re-check in Post() and not in CountDown(), we can guarantee
      // that both methods exit by checking whether the latch should be open.
      //
      // This means we can have a spurious open latch (which is easy to deal with), but
      // never a spurious closed latch (which blocks the thread and can't be dealt with).
      //
      previousCountdown = impl.Countdown.load(std::memory_order_consume);
      if(likely(previousCountdown == 0)) {
        __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
      }

    } // if(previousAdmitCounter < 0)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the latch counter. This locks the latch.
    std::size_t previousCountdown = impl.Countdown.fetch_add(
      count, std::memory_order_release
    );

    // If the latch was open at the time of this call, we need to close it so threads
    // can wait on the futex.
    if(unlikely(previousCountdown == 0)) {

      // There's a race condition at this point. If at this exact point, another thread
      // calls CountDown() and sets the FutexWord to 1 because the counter hit zero,
      // we'd falsely change it back to 0.
      //
      // Then a third thread which saw the countdown being greater than zero might reach
      // the futex wait and actually begin waiting even though the latch should be open.
      //
      // To fix this, we re-check the latch counter after setting the futex word.
      //
      if(likely(count > 0)) {
        impl.WaitWord = 0; // 0 -> latch now locked
        std::atomic_thread_fence(std::memory_order::memory_order_release);
      }

      // Re-check the latch counter. This might seem like a naive hack at first sight,
      // but by only doing this re-check in Post() and not in CountDown(), we can guarantee
      // that both methods exit by checking whether the latch should be open.
      //
      // This means we can have a spurious open latch (which is easy to deal with), but
      // never a spurious closed latch (which blocks the thread and can't be dealt with).
      //
      previousCountdown = impl.Countdown.load(std::memory_order_consume);
      if(likely(previousCountdown == 0)) {
        impl.WaitWord = 1; // 1 -> latch open
        std::atomic_thread_fence(std::memory_order::memory_order_release);
      }

    } // if(previousAdmitCounter < 0)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.Countdown.fetch_add(count, std::memory_order_release);

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Decrement the latch counter and fetch its previous value so we can both
    // detect when the counter goes negative and open the latch when it reaches zero
    std::size_t previousCountdown = impl.Countdown.fetch_sub(
      count, std::memory_order_release
    );
    assert((previousCountdown >= count) && u8"Latch remains zero or positive");

    // If we just decremented the latch to zero, signal the futex
    if(unlikely(previousCountdown == count)) {

      // Just like in the semaphore implementation, another thread may have incremented
      // the latch counter between our fetch_sub() and this point (a classical race
      // condition).
      //
      // This isn't a problem, however, as changing the futex wakes up all blocked threads
      // and causes them to re-check the counter. So we'll have potential spurious wake-ups,
      // but no spurious blocks.
      //
      if(likely(count > 0)) { // check needed? nobody would post 0 tasks...
        __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> countdown is zero
      }

      // Futex Wake (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This will signal other threads sitting in the Latch::Wait() method to re-check
      // the latch counter and resume running
      //
      Platform::LinuxFutexApi::PrivateFutexWakeAll(impl.FutexWord);

    } // if latch counter decremented to zero
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Decrement the latch counter and fetch its previous value so we can both
    // detect when the counter goes negative and open the latch when it reaches zero
    std::size_t previousCountdown = impl.Countdown.fetch_sub(
      count, std::memory_order_release
    );
    assert((previousCountdown >= count) && u8"Latch remains zero or positive");

    // If we just decremented the latch to zero, signal the futex
    if(unlikely(previousCountdown == count)) {

      // Just like in the semaphore implementation, another thread may have incremented
      // the latch counter between our fetch_sub() and this point (a classical race
      // condition).
      //
      // This isn't a problem, however, as changing the futex wakes up all blocked threads
      // and causes them to re-check the counter. So we'll have potential spurious wake-ups,
      // but no spurious blocks.
      //
      if(likely(count > 0)) { // check needed? nobody would post 0 tasks...
        impl.WaitWord = 1; // 1 -> countdown is zero
        std::atomic_thread_fence(std::memory_order::memory_order_release);
      }

      // WakeByAddressAll() (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddressall
      //
      // This will signal other threads sitting in the Latch::Wait() method to re-check
      // the latch counter and resume running
      //
      Platform::WindowsSyncApi::WakeByAddressAll(impl.WaitWord);

    } // if latch counter decremented to zero
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    {
      std::size_t previousCountdown = impl.Countdown.fetch_sub(
        count, std::memory_order_release
      );
      assert((previousCountdown >= count) && u8"Latch remains zero or positive");

      if(previousCountdown == count) {
        result = ::pthread_cond_signal(&impl.Condition);
        if(unlikely(result != 0)) {
          int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
          NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
          assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"Could not signal pthread conditional variable", result
          );
        }
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }

  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Loop until we find the latch to be open
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    for(;;) {
      if(safeCountdown == 0) {
        return;
      }

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0 // wait while futex word is 0 (== latch counter is greater than zero)
      );

      // If this was a spurious wake-up, we need to adjust the futex word in order to prevent
      // a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(safeCountdown > 0) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(safeCountdown == 0) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
        }
      }

    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Loop until we can snatch an available ticket
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    for(;;) {
      if(safeCountdown == 0) {
        return;
      }

      // WaitOnAddress (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
      //
      // This sends the thread to sleep for as long as the wait value has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::WindowsSyncApi::WaitOnAddress(
        static_cast<const volatile std::uint32_t &>(impl.WaitWord),
        static_cast<std::uint32_t>(0) // wait while wait variable is 0 (== gate closed)
      );

      // If this was a spurious wake-up, we need to adjust the wait variable in order to
      // prevent a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(safeCountdown > 0) {
        impl.WaitWord = 0; // 0 -> latch now locked
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(safeCountdown == 0) {
          impl.WaitWord = 1; // 1 -> latch open
          std::atomic_thread_fence(std::memory_order::memory_order_release);
        }
      }
    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.Countdown.load(std::memory_order_consume) > 0) {
      result = ::pthread_cond_wait(&impl.Condition, &impl.Mutex);
      if(unlikely(result != 0)) {
        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert(
          (unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler"
        );
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Obtain the starting time, but don't do anything with it yet (the futex
    // wait is relative, so unless we get EINTR, the time isn't even needed)
    struct ::timespec startTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &startTime);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get monotonic time for gate", errorNumber
      );
    }

    // Loop until we can snatch an available ticket
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    for(;;) {
      if(safeCountdown == 0) {
        return true;
      }

      // Calculate the remaining timeout until the wait should fail. Note that this is
      // a relative timeout (in contrast to ::sem_t and most things Posix).
      struct ::timespec timeout = Platform::PosixTimeApi::GetRemainingTimeout(
        CLOCK_MONOTONIC, startTime, patience
      );

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::WaitResult result = Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0,
        timeout
      );
      if(unlikely(result == Platform::LinuxFutexApi::TimedOut)) {
        return false; // Timeout elapsed, so it's time to give the bad news to the caller
      }

      // If this was a spurious wake-up, we need to adjust the futex word in order to prevent
      // a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(likely(safeCountdown > 0)) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(unlikely(safeCountdown == 0)) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
        }
      }
    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Query the tick counter, but don't do anything with it yet (the wait time is
    // relative, so unless we get a spurious wait, the tick counter isn't even needed)
    std::chrono::milliseconds startTickCount(::GetTickCount64());
    std::chrono::milliseconds patienceTickCount = (
      std::chrono::duration_cast<std::chrono::milliseconds>(patience)
    );
    std::chrono::milliseconds remainingTickCount = patienceTickCount;

    // Loop until we can snatch an available ticket
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    if(safeCountdown == 0) {
      return true;
    }
    for(;;) {

      // WaitOnAddress (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
      //
      // This sends the thread to sleep for as long as the wait value has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::WindowsSyncApi::WaitResult result = Platform::WindowsSyncApi::WaitOnAddress(
        static_cast<const volatile std::uint32_t &>(impl.WaitWord),
        static_cast<std::uint32_t>(0), // wait while wait variable is 0 (== gate closed)
        remainingTickCount
      );
      if(unlikely(result == Platform::WindowsSyncApi::WaitResult::TimedOut)) {
        return false;
      }

      // If this was a spurious wake-up, we need to adjust the futex word in order to prevent
      // a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(likely(safeCountdown > 0)) {
        impl.WaitWord = 0; // 0 -> latch now locked
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(unlikely(safeCountdown == 0)) {
          impl.WaitWord = 1; // 1 -> latch open
          std::atomic_thread_fence(std::memory_order::memory_order_release);
        }
      }
      if(safeCountdown == 0) {
        break;
      }

      // Calculate the new relative timeout. If this is some kind of spurious
      // wake-up, but the value does indeed change while we're here, that's not
      // a problem since the WaitOnAddress() call will re-check the wait value.
      {
        std::chrono::milliseconds elapsedTickCount = (
          std::chrono::milliseconds(::GetTickCount64()) - startTickCount
        );
        if(elapsedTickCount >= patienceTickCount) {
          return false; // timeout expired
        } else {
          remainingTickCount = patienceTickCount - elapsedTickCount;
        }
      }

    } // for(;;)

    return true; // wait noticed a change to the wait variable and latch was ready
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Forced to use CLOCK_REALTIME, which means the semaphore is broken :-(
    struct ::timespec endTime = Platform::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.Countdown.load(std::memory_order_consume) > 0) {
      result = ::pthread_cond_timedwait(&impl.Condition, &impl.Mutex, &endTime);
      if(unlikely(result != 0)) {
        if(result == ETIMEDOUT) {
          result = ::pthread_mutex_unlock(&impl.Mutex);
          if(unlikely(result != 0)) {
            Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
              u8"Could not unlock pthread mutex", result
            );
          }

          return false;
        }

        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert(
          (unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler"
        );
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }

    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const Latch::PlatformDependentImplementationData &Latch::getImplementationData() const {
    return *reinterpret_cast<const PlatformDependentImplementationData *>(
      this->implementationDataBuffer
    );
  }

  // ------------------------------------------------------------------------------------------- //

  Latch::PlatformDependentImplementationData &Latch::getImplementationData() {
    return *reinterpret_cast<PlatformDependentImplementationData *>(
      this->implementationDataBuffer
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
