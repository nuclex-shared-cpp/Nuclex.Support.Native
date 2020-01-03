#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Threading/Mutex.h"

#include <stdexcept>

#if defined(NUCLEX_SUPPORT_WIN32) || defined(NUCLEX_SUPPORT_WINRT)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#include <mutex>
#endif

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  #if defined(NUCLEX_SUPPORT_WIN32) || defined(NUCLEX_SUPPORT_WINRT)

  /// <summary>Mutex implementation for Windows systems</summary>
  struct Mutex::Implementation {

    /// <summary>Initializes a new Windows critical section</summary>
    public: Implementation() {
      ::InitializeCriticalSectionEx(&this->criticalSection, 4096, 0);
    }

    /// <summary>Destroys the Windows critical section</summary>
    public: ~Implementation() {
      ::DeleteCriticalSection(&this->criticalSection);
    }

    /// <summary>Enters the critical section</summary>
    public: void Lock() {
      ::EnterCriticalSection(&this->criticalSection);
    }

    /// <summary>Tries to enter the mutex</summary>
    /// <returns>True if the mutex was entered, false if it was occupied</returns>
    public: bool TryLock() {
     return (::TryEnterCriticalSection(&this->criticalSection) != FALSE);
    }

    /// <summary>Leaves the critical section again</summary>
    public: void Unlock() {
      ::LeaveCriticalSection(&this->criticalSection);
    }

    /// <summary>Critical section the mutex implementation is based on</summary>
    private: CRITICAL_SECTION criticalSection;

  };

  #else

  /// <summary>Mutex implementation for C++11</summary>
  struct Mutex::Implementation {

    /// <summary>Initializes a new C++11 mutex</summary>
    public: Implementation() {}

    /// <summary>Destroys the C++11 mutex</summary>
    public: ~Implementation() {}

    /// <summary>Enters the mutex</summary>
    public: void Lock() { this->mutex.lock(); }

    /// <summary>Tries to enter the mutex</summary>
    /// <returns>True if the mutex was entered, false if it was occupied</returns>
    public: bool TryLock() { return this->mutex.try_lock(); }

    /// <summary>Leavesthe mutex</summary>
    public: void Unlock() { this->mutex.unlock(); }

    /// <summary>C++11 mutex the implementation is based on</summary>
    private: std::mutex mutex;

  };

  #endif

  // ------------------------------------------------------------------------------------------- //

  Mutex::Mutex() : implementation(new Implementation()) {}

  // ------------------------------------------------------------------------------------------- //

  Mutex::~Mutex() { delete this->implementation; }

  // ------------------------------------------------------------------------------------------- //

  void Mutex::Lock() { this->implementation->Lock(); }

  // ------------------------------------------------------------------------------------------- //

  bool Mutex::TryLock() { return this->implementation->TryLock(); }

  // ------------------------------------------------------------------------------------------- //

  void Mutex::Unlock() { this->implementation->Unlock(); }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
