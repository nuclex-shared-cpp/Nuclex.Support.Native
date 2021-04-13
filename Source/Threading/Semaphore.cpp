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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/Semaphore.h"

#if defined(NUCLEX_SUPPORT_WIN32) // Use standard win32 threading primitives
#include "../Helpers/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#elif defined(NUCLEX_SUPPORT_LINUX) // Directly use futex via kernel syscalls
#include "Posix/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <linux/futex.h> // for futex constants
#include <unistd.h> // for ::syscall()
#include <limits.h> // for INT_MAX
#include <sys/syscall.h> // for ::SYS_futex
#include <atomic> // for std::atomic
#else // Posix: use a pthreads conditional variable to emulate a semaphore
#include "Posix/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <ctime> // for ::clock_gettime()
#include <atomic> // for std::atomic
#endif

#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32)
  // Just some safety checks to make sure pthread_condattr_setclock() is available.
  // https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
  //
  // You shouldn't encounter any Linux system where the Posix implementation isn't set
  // to Posix 2008-09 or something newer by default. If you do, you can set _POSIX_C_SOURCE
  // in your build system or remove the Semaphore implementation from your library.
  #if defined(_POSIX_C_SOURCE)
    #if (_POSIX_C_SOURCE < 200112L)
      #error Your C runtime library needs to at least implement Posix 2001-12 
    #endif
    //#if !defined(__USE_XOPEN2K)
  #endif
#endif

#include <cassert> // for assert()

// The situation on Linux/Posix systems is a bit depressing here:
//
// With ::sem_t, a native semaphore exists, but it always uses CLOCK_REALTIME which is
// prone to jumps (i.e. run ntp-client and it can easily jump seconds or minutes).
//
// There's a Bugzilla ticket for the kernel which hasn't changed status in 5 years:
// https://bugzilla.kernel.org/show_bug.cgi?id=112521
//
// And there's ::sem_timedwait_monotonic() on QNX, but not Posix:
// http://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/s/sem_timedwait.html
//
// On the other hand, Linux 2.6.28 makes its futexes use CLOCK_MONOTONIC by default
// https://man7.org/linux/man-pages/man2/futex.2.html
//
// Relying on ::sem_t is problematic. It works for a cron-style application where
// the wait is actually aiming for a time on the wall clock, but it's useless for
// normal thread synchronization where 50 ms may unexpectedly become 5 minutes or
// report ETIMEOUT after less than 1 ms.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Semaphore::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the semaphore</summary>
    /// <param name="initialCount">Initial admit count for the semaphore</param>
    public: PlatformDependentImplementationData(std::size_t initialCount);

    /// <summary>Frees all resources owned by the Semaphore</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_LINUX)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: volatile std::uint32_t FutexWord;
    /// <summary>Available tickets, negative for each thread waiting for a ticket</summary>
    public: std::atomic<int> AdmitCounter; 
#elif defined(NUCLEX_SUPPORT_WIN32)
    /// <summary>Handle of the semaphore used to pass or block threads</summary>
    public: HANDLE SemaphoreHandle;
#else // Posix
    /// <summary>How many threads the semaphore will admit</summary>
    public: std::atomic<std::size_t> AdmitCounter; 
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;

#endif

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    FutexWord(0),
    AdmitCounter(static_cast<int>(initialCount)) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    SemaphoreHandle(INVALID_HANDLE_VALUE) {

    // Figure out what the maximum number of threads passing through the semaphore
    // should be. We don't want a limit, but we also don't want to trigger some
    // undocumented special case code for the largest possible value...
    LONG maximumCount = std::numeric_limits<LONG>::max() - 10;

    // Create the Win32 event we'll use for this
    this->SemaphoreHandle = ::CreateSemaphoreW(
      nullptr, static_cast<LONG>(initialCount), maximumCount, nullptr
    );
    bool semaphoreCreationFailed = (
      (this->SemaphoreHandle == 0) || (this->SemaphoreHandle == INVALID_HANDLE_VALUE)
    );
    if(unlikely(semaphoreCreationFailed)) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not create semaphore for thread synchronization", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32) // -> Posix
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    AdmitCounter(initialCount),
    Condition(),
    Mutex() {

    // Attribute necessary to use CLOCK_MONOTONIC for condition variable timeouts
    ::pthread_condattr_t *monotonicClockAttribute = (
      Posix::PosixTimeApi::GetMonotonicClockAttribute()
    );
    
    // Create a new pthread conditional variable
    int result = ::pthread_cond_init(&this->Condition, monotonicClockAttribute);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable", result
      );
    }

    result = ::pthread_mutex_init(&this->Mutex, nullptr);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread mutex", result
      );
    }
  } 
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    // Nothing to do. If threads are waiting, they're now waiting on dead memory.
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    BOOL result = ::CloseHandle(this->SemaphoreHandle);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"Semaphore is closed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32) // -> Posix
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Semaphore::Semaphore(std::size_t initialCount) :
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // There will be a buffer overflow in the line after and the application will
    // likely crash or at least malfunction.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initialCount);
  }

  // ------------------------------------------------------------------------------------------- //

  Semaphore::~Semaphore() {
    getImplementationData().~PlatformDependentImplementationData();
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the semaphore admit counter so threads will be able to pass when woken up
    //__atomic_add_fetch(&impl.FutexWord, count, __ATOMIC_RELEASE);
    int previousAdmitCounter = impl.AdmitCounter.fetch_add(
      static_cast<int>(count), std::memory_order::memory_order_release
    );
    if(previousAdmitCounter < 0) { // If the semaphore has waiting threads
      int wakeupCount = -previousAdmitCounter;
      if(count < wakeupCount) {
        wakeupCount = count;
      }

      // If the semaphore was, but is no longer, contested, switch the futex word.
      //
      // This prevents a race condition for threads going to sleep (someone might
      // have posted work between their atomic fetch_sub() and the syscall).
      //
      // If that's the case, their syscall would return with EAGAIN because
      // the futex word has changed. In the worst case, we're waking all waiting
      // threads up for nothing, though...
      if(previousAdmitCounter + count >= 0) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE);
      }

      // Futex Wake (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This will signal other threads sitting in the Semaphore::WaitAndDecrement() method to
      // re-check the semaphore's status and resume running
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_WAKE_PRIVATE), // process-private futex wakeup
        static_cast<int>(wakeupCount), // wake up one thread for each uptick of the semaphore
        static_cast<struct ::timespec *>(nullptr), // timeout -> ignored
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        //__atomic_sub_fetch(&impl.FutexWord, count, __ATOMIC_RELEASE);
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wake up threads waiting on futex", errorNumber
        );
      }
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    BOOL result = ::ReleaseSemaphore(impl.SemaphoreHandle, static_cast<LONG>(count), nullptr);
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not increment semaphore", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32) // -> Posix
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.AdmitCounter.fetch_add(count, std::memory_order::memory_order_release);

    while(count > 0) {
      result = ::pthread_cond_signal(&impl.Condition);
      if(unlikely(result != 0)) {
        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not signal pthread conditional variable", result
        );
      }

      --count;
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Take one admit from the semaphore for this thread that was just let through
    //__atomic_sub_fetch(&impl.FutexWord, 1, __ATOMIC_SEQ_CST);
    int previousAdmitCounter = impl.AdmitCounter.fetch_sub(1, std::memory_order_seq_cst);
    if(previousAdmitCounter > 0) {
      return; // We were able to take a ticket lock-free
    }

    // If the semaphore just ran out of tickets, mark it as contested. This will
    // pointlessly wake up any current waiters (hopefully few, since state just switched)
    // but is necessary to avoid a race condition between the atomic fetch sub above
    // and our now necessary syscall.
    if(previousAdmitCounter == 0) {
      __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE);
    }

    // Be ready to check multiple times in case of EINTR
    for(;;) {

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<const volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_WAIT_PRIVATE), // process-private futex wakeup
        static_cast<int>(1), // wait while futex word is 0 (== no admits available)
        static_cast<struct ::timespec *>(nullptr), // timeout -> infinite
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(likely(errorNumber == EAGAIN)) {
          break; // Futex word changed -> semaphore is no longer contested!
        } else if(unlikely(errorNumber != EINTR)) {
          Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
            u8"Could not sleep on semaphore admit counter via futex wait", errorNumber
          );
        }
      } else {
        break;
      }

    } // for(;;)

    // TODO: Do we need to check something here?
    //   Should we atomic-load after EAGAIN and only exit if the semaphore is positive?

  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  void Semaphore::WaitThenDecrement() {
    const PlatformDependentImplementationData &impl = getImplementationData();

    DWORD result = ::WaitForSingleObject(impl.SemaphoreHandle, INFINITE);
    if(likely(result == WAIT_OBJECT_0)) {
      return;
    }

    DWORD lastErrorCode = ::GetLastError();
    Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
      u8"Error waiting for semaphore via WaitForSingleObject()", lastErrorCode
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32) // -> Posix
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.AdmitCounter.load(std::memory_order::memory_order_consume) == 0) {
      result = ::pthread_cond_wait(&impl.Condition, &impl.Mutex);
      if(unlikely(result != 0)) {
        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert(
          (unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler"
        );
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    impl.AdmitCounter.fetch_sub(1, std::memory_order::memory_order_release);

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience)  {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Take one admit from the semaphore for this thread that was just let through
    //__atomic_sub_fetch(&impl.FutexWord, 1, __ATOMIC_SEQ_CST);
    int previousAdmitCounter = impl.AdmitCounter.fetch_sub(1, std::memory_order_seq_cst);
    if(previousAdmitCounter > 0) {
      return true; // We were able to take a ticket lock-free
    }

    // If the semaphore just ran out of tickets, mark it as contested. This will
    // pointlessly wake up any current waiters (hopefully few, since state just switched)
    // but is necessary to avoid a race condition between the atomic fetch sub above
    // and our now necessary syscall.
    if(previousAdmitCounter == 0) {
      __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE);
    }

    // Query the time, but don't do anything with it yet (the futex wait is
    // relative, so unless we get EINTR, the time isn't even needed)
    struct ::timespec startTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &startTime);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get monotonic time for gate", errorNumber
      );
    }

    // Note that the timeout is a relative one
    //
    // From the docs:
    //   | Note: for FUTEX_WAIT, timeout is interpreted as a relative
    //   | value.  This differs from other futex operations, where
    //   | timeout is interpreted as an absolute value.
    struct ::timespec timeout;
    {
      const std::size_t NanoSecondsPerMicrosecond = 1000; // 1,000 ns = 1 μs

      // timespec has seconds and nanoseconds, so divide the microseconds into full seconds
      // and remainder milliseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(patience.count(), 1000000);
      timeout.tv_sec = divisionResults.quot;
      timeout.tv_nsec = divisionResults.rem * NanoSecondsPerMicrosecond;
    }

    // Check the futex word and wait on it until it changes. Normally, this loops exactly
    // once, but EINTR may still happen and require us to recalculate the relative timeout.
    for(;;) {

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<const volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_PRIVATE_FLAG | FUTEX_WAIT), // process-private futex wakeup
        static_cast<int>(1), // wait while futex word is 0 (== no admits available)
        static_cast<struct ::timespec *>(&timeout), // timeout after which to fail
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(likely(errorNumber == EAGAIN)) {
          break; // Futex word changed -> semaphore is no longer contested!
        } else if(likely(errorNumber == ETIMEDOUT)) { // Timeout, wait failed
          previousAdmitCounter = impl.AdmitCounter.fetch_add(
            1, std::memory_order::memory_order_release
          );       
          if(previousAdmitCounter == -1) {
            __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE);
          }
          return false;
        } else if(unlikely(errorNumber != EINTR)) {
          Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
            u8"Could not sleep on gate status via futex wait", errorNumber
          );
        }
      } else { // result did not indicate an error, so the futex word has changed!
        break;
      }

      // Calculate the new relative timeout. If this is some kind of spurious
      // wake-up, but the value does indeed change while we're here, that's not
      // a problem since the futex syscall will re-check the futex word.
      timeout = Posix::PosixTimeApi::GetRemainingTimeout(CLOCK_MONOTONIC, startTime, patience);

    }

    return true; // wait noticed a change to the futex word
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience)  {
    PlatformDependentImplementationData &impl = getImplementationData();

    DWORD milliseconds = static_cast<DWORD>(patience.count() + 500 / 1000);
    DWORD result = ::WaitForSingleObject(impl.SemaphoreHandle, milliseconds);
    if(likely(result == WAIT_OBJECT_0)) {
      return true;
    } else if(result == WAIT_TIMEOUT) {
      return false;
    }

    DWORD lastErrorCode = ::GetLastError();
    Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
      u8"Error waiting for semaphore via WaitForSingleObject()", lastErrorCode
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WIN32) // -> Posix
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Forced to use CLOCK_REALTIME, which means the semaphore is broken :-(
    struct ::timespec endTime = Posix::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.AdmitCounter.load(std::memory_order::memory_order_consume) == 0) {
      result = ::pthread_cond_timedwait(&impl.Condition, &impl.Mutex, &endTime);
      if(unlikely(result != 0)) {
        if(result == ETIMEDOUT) {
          result = ::pthread_mutex_unlock(&impl.Mutex);
          if(unlikely(result != 0)) {
            Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
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
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    impl.AdmitCounter.fetch_sub(1, std::memory_order::memory_order_release);

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }

    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Semaphore::PlatformDependentImplementationData &Semaphore::getImplementationData() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
