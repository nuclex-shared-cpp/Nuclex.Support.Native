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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Configuration options for the thread pool</summary>
  class ThreadPoolConfig {

    /// <summary>Maximum size of a submitted task to be re-used via the pool</summary>
    /// <remarks>
    ///   <para>
    ///     The thread pool packages all information about a task (method pointer,
    ///     passed arguments, internal bookkeeping) into heap-allocated memory blocks
    ///     that it attempts to reuse in future tasks.
    ///   </para>
    ///   <para>
    ///     This is the size limit, in bytes, beyond which a task's memory block is not
    ///     reused but freed immediately after the task is finished.
    ///   </para>
    ///   <para>
    ///     Otherwise, if the user submits a gigantic task, even just once in a while,
    ///     it would enter the reuse pool and also get re-used for smaller tasks,
    ///     requiring another allocation when another gigantic task is scheduled.
    ///     Eventually, only oversized memory blocks would be circulating around.
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t SubmittedTaskReuseLimit = 128;

    /// <summary>Once per how many milliseconds each worker thread wakes up</summary>
    /// <remarks>
    ///   <para>
    ///     Worker threads are immediately woken up through a semaphore if there is work
    ///     to do or if the thread pool is shutting down.
    ///   </para>
    ///   <para>
    ///     If worker threads are idle, however, they will once in a while check if
    ///     they can shut down (until the minimumThreadCount parameter is reached).
    ///     This interval specifies, how often threads will check if they can shut down
    ///     and look for hanging work (the latter should never be the case, but as
    ///     a matter of defensive programming, it is done anyway).
    ///   </para>
    ///   <para>
    ///     Should work be issued at a faster rate than the heart beat interval,
    ///     then this value has no effect. With a value of 50 milliseconds, if you
    ///     generate work (for all threads) at 20 fps, the threads will always
    ///     wake up for work and never due to an idle heartbeat.
    ///   </para>
    ///   <para>
    ///     This value is only used by the Linux implementation of the thread pool
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t WorkerHeartBeatMilliseconds = 50;

    /// <summary>Number of heartbeats after which a thread tries shutting down</summary>
    /// <remarks>
    ///   <para>
    ///     If a thread has consecutively woken this number of times due to having
    ///     no work, it will terminate unless the thread pool already is at
    ///     the minimum number of threads specified during construction.
    ///   </para>
    ///   <para>
    ///     This value is only used by the Linux implementation of the thread pool
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t IdleShutDownHeartBeats = 10;


  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H
