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

#ifndef NUCLEX_SUPPORT_INTEROP_POSIXTIMEAPI_H
#define NUCLEX_SUPPORT_INTEROP_POSIXTIMEAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "PosixApi.h"

#include <cassert> // for assert()
#include <chrono> // for std::chrono::microseconds, std::chrono::milliseconds

#include <sys/time.h> // for ::timespec

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix time API</summary>
  class PosixTimeApi {

    /// <summary>
    ///   Returns a time point that lies the specified number if microseconds in the future
    /// </summary>
    /// <param name="clock">Which clock to query (i.e. <code>CLOCK_MONOTONIC</code>)</param>
    /// <param name="addedTime">Amount of microseconds that will be added</param>
    /// <returns>
    ///   A time point that lies the specified number of microsends in the future from
    ///   the moment the method is called.
    /// </returns>
    public: static struct ::timespec GetTimePlus(
      ::clockid_t clock, std::chrono::microseconds addedTime
    );

    /// <summary>
    ///   Returns a time point that lies the specified number if milliseconds in the future
    /// </summary>
    /// <param name="clock">Which clock to query (i.e. <code>CLOCK_MONOTONIC</code>)</param>
    /// <param name="addedTime">Amount of milliseconds that will be added</param>
    /// <returns>
    ///   A time point that lies the specified number of milliseconds in the future from
    ///   the moment the method is called.
    /// </returns>
    public: static struct ::timespec GetTimePlus(
      ::clockid_t clock, std::chrono::milliseconds addedTime
    );

    /// <summary>
    ///   Calculates the remaining relative timeout from the current clock time
    /// </summary>
    /// <param name="clock">Clock on which the timeout is being counted down</param>
    /// <param name="startTime">Time at which the timeout began</param>
    /// <param name="timeout">Timeout that is being counted down</param>
    /// <returns>
    ///   The remaining (relative) time until the timeout. Will return zero if
    ///   the timeout has already elapsed and never a negative time.
    /// </returns>
    public: static struct ::timespec GetRemainingTimeout(
      ::clockid_t clock,
      const struct ::timespec &startTime,
      std::chrono::microseconds timeout
    );

    /// <summary>Checks whether the specified end time has been reached yet</summary>
    /// <param name="clock">Clock against which the end time will be checked</param>
    /// <param name="endTime">Time after which this method will return true</param>
    /// <returns>
    ///   True if the current time is equal to or later than the specified end time,
    ///   false if the current time is still earlier.
    /// </returns>
    public: static bool HasTimedOut(::clockid_t clock, const struct ::timespec &endTime);

    /// <summary>
    ///   Returns a pthread conditional variable attribute that lets the conditional variable
    ///   use CLOCK_MONOTONIC instead of CLOCK_REALTIME
    /// </summary>
    /// <returns>A pre-configured conditional variable attribute</returns>
    public: static ::pthread_condattr_t *GetMonotonicClockAttribute();

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_INTEROP_POSIXTIMEAPI_H
