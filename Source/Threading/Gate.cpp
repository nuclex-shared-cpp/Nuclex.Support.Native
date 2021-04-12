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

#include "Nuclex/Support/Threading/Gate.h"

#if defined(NUCLEX_SUPPORT_WIN32)
#include "../Helpers/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#else
#include "Posix/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <ctime> // for ::clock_gettime()
#include <atomic> // for std::atomic

#if defined(NUCLEX_SUPPORT_LINUX)
// Just some safety checks to make sure pthread_condattr_setclock() is available.
// https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
//
// You shouldn't encounter any Linux system where the Posix implementation isn't set
// to Posix 2008-09 or something newer by default. If you do, you can set _POSIX_C_SOURCE
// in your build system or remove the Gate implementation from your library.
#if defined(_POSIX_C_SOURCE)
  #if (_POSIX_C_SOURCE < 200112L)
    #error Your C runtime library needs to at least implement Posix 2001-12 
  #endif
  //#if !defined(__USE_XOPEN2K)
#endif

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // defined(NUCLEX_SUPPORT_WIN32)... else

#include <cassert> // for assert()

// The Linux kernel's futex syscalls have support for CLOCK_MONOTONIC built in.
// https://man7.org/linux/man-pages/man2/futex.2.html
//
// It would be cool if, in addition to a Posix implementation, I could create
// a Linux-only implementation that circumvents pthreads and talks directly
// to the kernel to implement a gate.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Gate::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the gate</summary>
    /// <param name="initiallyOpen">Whether the gate is initially open</param>
    public: PlatformDependentImplementationData(bool initiallyOpen);

    /// <summary>Frees all resources owned by the gate</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_WIN32)
    /// <summary>Handle of the event used to pass or block threads</summary>
    public: HANDLE EventHandle;
#else

    // Some implementations have a ::pthread_mutex_timedlock_monotonic

    /// <summary>Whether the gate is currently open</summary>
    public: std::atomic<bool> IsOpen; 
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
    
#endif

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  Gate::PlatformDependentImplementationData::PlatformDependentImplementationData(
    bool initiallyOpen
  ) :
    EventHandle(INVALID_HANDLE_VALUE) {

    // Create the Win32 event we'll use for this
    this->EventHandle = ::CreateEventW(nullptr, TRUE, initiallyOpen ? TRUE : FALSE, nullptr);
    bool eventCreationFailed = (
      (this->EventHandle == 0) || (this->EventHandle == INVALID_HANDLE_VALUE)
    );
    if(unlikely(eventCreationFailed)) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not create thread synchronication event for gate", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  Gate::PlatformDependentImplementationData::PlatformDependentImplementationData(
    bool initiallyOpen
  ) :
    IsOpen(initiallyOpen),
    Condition() {

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
#if defined(NUCLEX_SUPPORT_WIN32)
  Gate::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    BOOL result = ::CloseHandle(this->EventHandle);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"Synchronization event is closed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  Gate::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Gate::Gate(bool initiallyOpen) :
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // Things will still work, but we have to resort to an extra allocation.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initiallyOpen);
  }

  // ------------------------------------------------------------------------------------------- //

  Gate::~Gate() {
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
  void Gate::Open() {
    PlatformDependentImplementationData &impl = getImplementationData();

    DWORD result = ::SetEvent(impl.EventHandle);
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not set synchronization event to signaled state", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  void Gate::Open() {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.IsOpen.store(true, std::memory_order::memory_order_relaxed);
    result = ::pthread_cond_signal(&impl.Condition);
    if(unlikely(result != 0)) {
      int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
      assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not signal pthread conditional variable", result
      );
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
#if defined(NUCLEX_SUPPORT_WIN32)
  void Gate::Close() {
    PlatformDependentImplementationData &impl = getImplementationData();

    DWORD result = ::ResetEvent(impl.EventHandle);
    if(result == FALSE) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not set synchronization event to non-signaled state", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  void Gate::Close() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // We don't need memory_order_release, but the caller is likely to expect a fence
    impl.IsOpen.store(false, std::memory_order::memory_order_release);
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  void Gate::Set(bool opened) {
    if(opened) {
      Open();
    } else {
      Close();
    }
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  void Gate::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    DWORD result = ::WaitForSingleObject(impl.EventHandle, INFINITE);
    if(likely(result == WAIT_OBJECT_0)) {
      return;
    }

    DWORD lastErrorCode = ::GetLastError();
    Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
      u8"Error waiting for sychronization event via WaitForSingleObject()", lastErrorCode
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  void Gate::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(!impl.IsOpen.load(std::memory_order::memory_order_consume)) {
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

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  bool Gate::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    DWORD milliseconds = static_cast<DWORD>(patience.count() + 500 / 1000);
    DWORD result = ::WaitForSingleObject(impl.EventHandle, milliseconds);
    if(likely(result == WAIT_OBJECT_0)) {
      return true;
    } else if(result == WAIT_TIMEOUT) {
      return false;
    }

    DWORD lastErrorCode = ::GetLastError();
    Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
      u8"Error waiting for sychronization event via WaitForSingleObject()", lastErrorCode
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  bool Gate::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    struct ::timespec waitEndTime = Posix::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthreads mutex", result
      );
    }

    while(!impl.IsOpen.load(std::memory_order::memory_order_consume)) {
      result = ::pthread_cond_timedwait(&impl.Condition, &impl.Mutex, &waitEndTime);
      if(unlikely(result != 0)) {
        if(result == ETIMEDOUT) {
          result = ::pthread_mutex_unlock(&impl.Mutex);
          if(unlikely(result != 0)) {
            Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
              u8"Could not unlock pthreads mutex", result
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
          u8"Could not wait on pthreads conditional variable", result
        );
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthreads mutex", result
      );
    }
    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const Gate::PlatformDependentImplementationData &Gate::getImplementationData() const {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<const PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Gate::PlatformDependentImplementationData &Gate::getImplementationData() {
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
