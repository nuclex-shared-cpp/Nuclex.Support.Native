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
#include "../Helpers/WindowsApi.h" // for ::Sleep(), ::GetCurrentThreadId() and more
#else
#include "Posix/PosixProcessApi.h" // for PosixProcessApi
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
    #error Your C runtime library enedsto at least implement Posix 2001-12 
  #endif
  //#if !defined(__USE_XOPEN2K)
#endif

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // defined(NUCLEX_SUPPORT_WIN32)... else

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  /// <summary>A condition attribute that switches timeouts to the monotonic clock</summary>
  /// <remarks>
  ///   By default, timeouts run on the REALTIME clock (historic Posix, I guess), which
  ///   would risk all wait functions either skipping their wait or waiting for over an hour
  ///   when the system clock changes due to daylight savings time.
  /// </remarks>
  class MonotonicClockConditionAttribute {

    /// <summary>Initializes the monotonic clock attribute</summary>
    public: MonotonicClockConditionAttribute();

    /// <summary>Destroys the attribute</summary>
    public: ~MonotonicClockConditionAttribute();

    /// <summary>Accesses the conditional variable attribute</summary>
    /// <returns>The address of the attribute which can be passed to pthread functions</returns>
    public: ::pthread_condattr_t *GetAttribute() const {
      return &this->attribute;
    }

    /// <summary>Conditional variable attributes managed by this instance</summary>
    private: ::pthread_condattr_t attribute;

  };
#endif // !defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  MonotonicClockConditionAttribute::~MonotonicClockConditionAttribute() {

    int result = ::pthread_condattr_destroy(&this->attribute);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable attribute is destroyed");

  }
#endif // !defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  MonotonicClockConditionAttribute::MonotonicClockConditionAttribute() {

    // Initialize the conditional attribute structure
    int result = ::pthread_condattr_init(&this->attribute);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable attribute", result
      );
    }

    // Change the attribute's clock settings so the monotonic clock is used
    result = ::pthread_condattr_setclock(&this->attribute, CLOCK_MONOTONIC);
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not set pthread conditional variable attribute's clock id", result
      );
    }

  }
#endif // !defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

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
    /// <summary>Retrieves a pthreads attribute that selects the monotonic clock</summary>
    /// <returns>A pthread conditional variable attribute selecting the monotonic clock</returns>
    public: static ::pthread_condattr_t *getMonotonicClockAttribute() {
      static MonotonicClockConditionAttribute attributeContainer;
      return attributeContainer.GetAttribute();
    }

    /// <summary>Whether the gate is currently open</summary>
    public: std::atomic<bool> IsOpen; 
    /// <summary>The pthread conditional variable used to pass or block threads</summary>
    public: ::pthread_cond_t Condition;
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
    
    // Create a new pthread conditional variable
    int result = ::pthread_cond_init(&this->Condition, getMonotonicClockAttribute());
    if(unlikely(result != 0)) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable", result
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

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
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
    
    bool isOpen = impl.IsOpen.load(std::memory_order::memory_order_consume);
    if(isOpen) {
      return;
    } else {

    }
    ::pthread_cond_signal()
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
