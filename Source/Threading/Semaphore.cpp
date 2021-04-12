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

#if defined(NUCLEX_SUPPORT_WIN32)
#include "../Helpers/WindowsApi.h" // for ::CreateSemaphoewR(), CloseHandle() and more
#include <limits> // for std::numeric_limits
#else
#include "Posix/PosixTimeApi.h" // for PosixTimeApi
#include <ctime> // for ::clock_gettime()
#include <semaphore.h> // for ::sem_init(), ::sem_wait(), ::sem_post(), ::sem_destroy()
#include <atomic> // for std::atomic
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
// And there's ::stm_timedwait_monotonic() on QNX, but not Posix:
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
// Perhaps here, too, interfacing directly with the Linux kernel's futex syscalls
// could save a lot of bloat and uncertainty.
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

#if defined(NUCLEX_SUPPORT_WIN32)
    /// <summary>Handle of the semaphore used to pass or block threads</summary>
    public: HANDLE SemaphoreHandle;
#else
    public: ::sem_t Semaphore; // <--  not usable for anything but cron-type apps
/*
    /// <summary>How many threads the semaphore will admit</summary>
    public: std::atomic<std::size_t> AdmitCounter; 
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
*/
#endif

  };

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
#if !defined(NUCLEX_SUPPORT_WIN32)
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    Semaphore() /*,
    AdmitCounter(initialCount),
    Condition(),
    Mutex() */{

    int result = ::sem_init(&this->Semaphore, 0, static_cast<int>(initialCount));
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not create semaphore", errorNumber
      );
    }
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
#if !defined(NUCLEX_SUPPORT_WIN32)
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::sem_destroy(&this->Semaphore);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"Semaphore is successfully destroyed");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Semaphore::Semaphore(std::size_t initialCount) :
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // Things will still work, but we have to resort to an extra allocation.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initialCount);
  }

  // ------------------------------------------------------------------------------------------- //

  Semaphore::~Semaphore() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      getImplementationData().~PlatformDependentImplementationData();
    } else {
      delete this->implementationData;
    }
  }

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
#if !defined(NUCLEX_SUPPORT_WIN32)
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    while(count > 0) {
      int result = ::sem_post(&impl.Semaphore);
      if(result == -1) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not increment semaphore", errorNumber
        );
      }

      --count;
    }
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
#if !defined(NUCLEX_SUPPORT_WIN32)
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::sem_wait(&impl.Semaphore);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not wait on and decrement semaphore", errorNumber
      );
    }
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
#if !defined(NUCLEX_SUPPORT_WIN32)
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Forced to use CLOCK_REALTIME, which means the semaphore is broken :-(
    struct ::timespec endTime = Posix::PosixTimeApi::GetTimePlus(CLOCK_REALTIME, patience);

    int result = ::sem_timedwait(&impl.Semaphore, &endTime);
    if(result == -1) {
      int errorNumber = errno;
      if(errorNumber == ETIMEDOUT) {
        return false;
      }
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not wait on and decrement semaphore", errorNumber
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
