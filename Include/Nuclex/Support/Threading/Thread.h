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

#ifndef NUCLEX_SUPPORT_THREADING_THREAD_H
#define NUCLEX_SUPPORT_THREADING_THREAD_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for for std::uintptr_t
#include <chrono> // for std::chrono::microseconds etc.
#include <thread> // for std::thread

namespace Nuclex::Support::Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides supporting methods for threads</summary>
  /// <remarks>
  ///   <para>
  ///     The thread affinity methods provided by this class are limited to 64 CPUs and
  ///     do not provide any methods for querying NUMA nodes (i.e. systems where CPUs are
  ///     provided by two or more physical chips). For situations where extreme thread
  ///     utilization is needed (i.e. AI, raytracing, containers shared among large numbers
  ///     of threads), please use pthreads, libnuma or a portable wrapper.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE Thread {

    /// <summary>Lets the calling thread wait for the specified amount of time</summary>
    /// <param name="time">Duration for which the thread will wait</param>
    public: NUCLEX_SUPPORT_API static void Sleep(std::chrono::microseconds time);

    /// <summary>Determines whether the calling thread belongs to the thread pool</summary>
    /// <returns>True if the calling thread belongs to the thread pool</returns>
    public: NUCLEX_SUPPORT_API static bool BelongsToThreadPool();

    // This method is not cleanly implementable on Microsoft platforms
#if defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
    /// <summary>Returns a unique ID for the calling thread</summary>
    /// <returns>
    ///   A unique ID that no other thread that's running at the same time will have
    /// </returns>
    /// <remarks>
    ///   This is useful for some lock-free synchronization techniques. It is also used
    ///   as the input to the thread affinity setting methods.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uintptr_t GetCurrentThreadId();
#endif //defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)

    /// <summary>Returns a unique ID for the specified thread</summary>
    /// <returns>
    ///   The unique ID for the specified thread which no other thread that's running at
    ///   the same time will have
    /// </returns>
    /// <remarks>
    ///   This is useful for some lock-free synchronization techniques. It is also used
    ///   as the input to the thread affinity setting methods.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uintptr_t GetStdThreadId(std::thread &thread);

    /// <summary>Checks which CPU cores a thread is allowed to run on</summary>
    /// <param name="threadId">Thread whose CPU affinity mask will be retrieved</param>
    /// <returns>A bit mask where each bit corresponds to a CPU core</returns>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on. So unless you change
    ///     a thread's affinity, this will return a mask of all CPU cores available.
    ///   </para>
    ///   <para>
    ///     See <see cref="SetCpuAffinityMask" /> for a short description of why you may
    ///     or may not want to adjust CPU affinity for a thread.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uint64_t GetCpuAffinityMask(std::uintptr_t threadId);

    /// <summary>Checks which CPU cores the calling thread is allowed to run on</summary>
    /// <returns>A bit mask where each bit corresponds to a CPU core</returns>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on. So unless you change
    ///     a thread's affinity, this will return a mask of all CPU cores available.
    ///   </para>
    ///   <para>
    ///     See <see cref="SetCpuAffinityMask" /> for a short description of why you may
    ///     or may not want to adjust CPU affinity for a thread.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uint64_t GetCpuAffinityMask();

    /// <summary>Selects the CPU cores on which a thread is allowed to run</summary>
    /// <param name="threadId">ID of the thread whose CPU affinity mask will be changed</param>
    /// <param name="affinityMask">Bit mask of CPU cores the thread can run on</param>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on.
    ///   </para>
    ///   <para>
    ///     In most cases, it is a good idea to leave it that way - for low-thread
    ///     operations, the CPU core is often cycled to ensure heat is generated evenly
    ///     over the whole chip, allowing &quot;TurboBoost&quot; (Intel),
    ///     &quot;TurboCore&quot; (AMD) to raise clock frequencies.
    ///   </para>
    ///   <para>
    ///     For highly threaded operations on the other hand it can make sense to assign
    ///     them to fixed CPU cores. For example, to keep a UI or communications thread
    ///     unclogged, or to optimize performance on NUMA systems (actual multi-CPU systems
    ///     have one memory controller per chip, so if multiple chips massage the same
    ///     memory area, expensive synchronization between the memory controllers via
    ///     the system bus needs to happen).
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void SetCpuAffinityMask(
      std::uintptr_t threadId, std::uint64_t affinityMask
    );

    /// <summary>Selects the CPU cores on which the calling thread is allowed to run</summary>
    /// <param name="affinityMask">Bit mask of CPU cores the thread can run on</param>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on.
    ///   </para>
    ///   <para>
    ///     In most cases, it is a good idea to leave it that way - for low-thread
    ///     operations, the CPU core is often cycled to ensure heat is generated evenly
    ///     over the whole chip, allowing &quot;TurboBoost&quot; (Intel),
    ///     &quot;TurboCore&quot; (AMD) to raise clock frequencies.
    ///   </para>
    ///   <para>
    ///     For highly threaded operations on the other hand it can make sense to assign
    ///     them to fixed CPU cores. For example, to keep a UI or communications thread
    ///     unclogged, or to optimize performance on NUMA systems (actual multi-CPU systems
    ///     have one memory controller per chip, so if multiple chips massage the same
    ///     memory area, expensive synchronization between the memory controllers via
    ///     the system bus needs to happen).
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void SetCpuAffinityMask(std::uint64_t affinityMask);

    private: Thread(const Thread &) = delete;
    private: Thread&operator =(const Thread &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
