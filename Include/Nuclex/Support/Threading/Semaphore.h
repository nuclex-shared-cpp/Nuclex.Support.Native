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

#ifndef NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
#define NUCLEX_SUPPORT_THREADING_SEMAPHORE_H

#include "Nuclex/Support/Config.h"

// https://softwareengineering.stackexchange.com/questions/340284/mutex-vs-semaphore-how-to-implement-them-not-in-terms-of-the-other
//#undef NUCLEX_SUPPORT_LINUX

#include <cstddef> // for std::size_t
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets only a specific number of threads enter at the same time</summary>
  /// <remarks>
  ///   <para>
  ///   </para>
  /// </remarks>
  class Semaphore {

    /// <summary>Initializes a new semaphore with the specified initial count</summary>
    /// <param name="initialCount">
    ///   Initial number of threads that the semaphore will let through
    /// </param>
    public: NUCLEX_SUPPORT_API Semaphore(std::size_t initialCount = 0);

    /// <summary>Frees all resources owned by the semaphore</summary>
    /// <remarks>
    ///   There should not be any threads waiting on the semaphore when it is destroyed.
    ///   The behavior for such threats is undefined, they may hang forever, they
    ///   may receive an exception or the entire process may be terminated.
    /// </remarks>
    public: NUCLEX_SUPPORT_API ~Semaphore();

    /// <summary>Increments the semaphore, letting one more thread through</summary>
    /// <param name="count">Number of times the semaphore will be incremented</param>
    public: NUCLEX_SUPPORT_API void Post(std::size_t count = 1);

    /// <summary>
    ///   Waits until the semaphore has a count above zero, then decrements the count
    /// </summary>
    /// <remarks>
    ///   This caues the calling thread to block if the semaphore didn't already have
    ///   a positive count. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Post" /> on the semaphore.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void WaitThenDecrement();

    /// <summary>
    ///   Waits until the semaphore has a count above zero, then decrements the count
    /// </summary>
    /// <param name="patience">How long to wait for the semaphore before giving up</parma>
    /// <returns>
    ///   True if the semaphore let the thread through and was decremented,
    ///   false if the timeout elapsed and the semaphore was not decremented.
    /// <returns>
    /// <remarks>
    ///   This caues the calling thread to block if the semaphore didn't already have
    ///   a positive count. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Post" /> on the semaphore or until the specified
    ///   patience time has elapsed.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool WaitForThenDecrement(
      const std::chrono::microseconds &patience
    );

    //public: void WaitUntilThenDecrement(const std::chrono::time_point< &patience);

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
    private: union alignas(8) {
      /// <summary>Platform dependent process and file handles used for the process</summary>
      PlatformDependentImplementationData *implementationData;
      /// <summary>Used to hold the platform dependent implementation data if it fits</summary>
      /// <remarks>
      ///   Small performance / memory fragmentation improvement.
      ///   This avoids a micro-allocation for the implenmentation data structure in most cases.
      /// </remarks>
#if defined(NUCLEX_SUPPORT_LINUX)
      unsigned char implementationDataBuffer[16];
#elif defined(NUCLEX_SUPPORT_WIN32)
      unsigned char implementationDataBuffer[sizeof(std::size_t)]; // matches HANDLE size
#else // Posix
      unsigned char implementationDataBuffer[96];
#endif
    };

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
