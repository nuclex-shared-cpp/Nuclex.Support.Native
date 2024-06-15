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

#ifndef NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
#define NUCLEX_SUPPORT_THREADING_SEMAPHORE_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets only a specific number of threads enter at the same time</summary>
  /// <remarks>
  ///   <para>
  ///     This is a completely vanilla semaphore implementation that either delegates
  ///     to the platform's threading library or uses available threading primitives to
  ///     build the required behavior.
  ///   </para>
  ///   <para>
  ///     Using it grants you automatic resource management, reduced header dependencies
  ///     and guaranteed behavior, including actual relative timeouts on Posix platforms
  ///     where the default implementation would use wall clock (meaning clock adjustment
  ///     sensitive) timeouts.
  ///   </para>
  ///   <para>
  ///     It's at least as fast as your platform's native semaphore, likely much faster.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE Semaphore {

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
    /// <param name="patience">How long to wait for the semaphore before giving up</param>
    /// <returns>
    ///   True if the semaphore let the thread through and was decremented,
    ///   false if the timeout elapsed and the semaphore was not decremented.
    /// <returns>
    /// <remarks>
    ///   This causes the calling thread to block if the semaphore didn't already have
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
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)
    alignas(8) unsigned char implementationDataBuffer[sizeof(std::size_t) * 2];
#else // Posix
    unsigned char implementationDataBuffer[96];
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
