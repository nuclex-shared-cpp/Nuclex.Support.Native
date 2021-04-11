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
#include "../Helpers/WindowsApi.h" // for ::Sleep(), ::GetCurrentThreadId() and more
#include <limits>
#else
#include "Posix/PosixProcessApi.h" // for PosixProcessApi
#include <ctime> // for ::clock_gettime()
#include <atomic> // for std::atomic
#endif

#include <cassert> // for assert()

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
    /// <summary>Retrieves a pthreads attribute that selects the monotonic clock</summary>
    /// <returns>A pthread conditional variable attribute selecting the monotonic clock</returns>
    public: static ::pthread_condattr_t *getMonotonicClockAttribute() {
      static MonotonicClockConditionAttribute attributeContainer;
      return attributeContainer.GetAttribute();
    }

    /// <summary>Whether the Semaphore is currently open</summary>
    public: std::atomic<bool> IsOpen; 
    /// <summary>The pthread conditional variable used to pass or block threads</summary>
    public: ::pthread_cond_t Condition;
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
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    BOOL result = ::CloseHandle(this->SemaphoreHandle);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"Semaphore is closed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
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
    const PlatformDependentImplementationData &impl = getImplementationData();
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
    const PlatformDependentImplementationData &impl = getImplementationData();

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
