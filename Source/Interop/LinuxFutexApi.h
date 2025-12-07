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

#ifndef NUCLEX_SUPPORT_INTEROP_LINUXFUTEXAPI_H
#define NUCLEX_SUPPORT_INTEROP_LINUXFUTEXAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include <string> // std::string
#include <cstdint> // std::uint8_t and std::size_t

#include <ctime> // for ::timespec

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Linux futex synchronization API</summary>
  class LinuxFutexApi {

    // ----------------------------------------------------------------------------------------- //
    // These are all for "private" futexes. Which means we hint to the Linux kernel
    // that the futex is private to the calling process (i.e. not in shared memory)
    // and certain assumptions and optimizations for that special case can be made.
    // ----------------------------------------------------------------------------------------- //

    #pragma region enum WaitResult

    /// <summary>Reasons for why <see cref="WaitOnAddress" /> has returned</summary>
    public: enum WaitResult {

      /// <summary>The wait was cancelled because the timeout was reached</summary>>
      TimedOut = -1,
      /// <summary>The wait was interrupted for some reason</summary>
      Interrupted = 0,
      /// <summary>Either the monitored value changed or we woke spuriously</summary>
      ValueChanged = 1

      // We can distinguish between ValueChanged and ManualWake, but we don't need it

    };

    #pragma endregion // enum WaitResult

    /// <summary>Waits for a private futex variable to change its value</summary>
    /// <param name="futexWord">Futex word that will be watched for changed</param>
    /// <param name="comparisonValue">
    ///   Value the futex word is expected to have, the method will return immediately
    ///   when the watched futex word has a different value.
    /// </param>
    /// <returns>
    ///   The reason why the wait method has returned. This method will never report back
    ///   <see cref="WaitResult::TimedOut" /> as a reason because it does not time out.
    /// </returns>
    public: static WaitResult PrivateFutexWait(
      const volatile std::uint32_t &futexWord,
      std::uint32_t comparisonValue
    );

    /// <summary>Waits for a private futex variable to change its value</summary>
    /// <param name="futexWord">Futex word that will be watched for changed</param>
    /// <param name="comparisonValue">
    ///   Value the futex word is expected to have, the method will return immediately
    ///   when the watched futex word has a different value.
    /// </param>
    /// <param name="patience">
    ///   Maximum amount of time to wait before returning even when the value doesn't change
    /// </param>
    /// <returns>The reason why the wait method has returned</returns>
    public: static WaitResult PrivateFutexWait(
      const volatile std::uint32_t &futexWord,
      std::uint32_t comparisonValue,
      const ::timespec &patience
    );

    /// <summary>Wakes a single thread waiting for a futex word to change</summary>
    /// <param name="futexWord">Futex word that is being watched by threads</param>
    public: static void PrivateFutexWakeSingle(
      const volatile std::uint32_t &futexWord
    );

    /// <summary>Wakes all threads waiting for a futex word to change</summary>
    /// <param name="futexWord">Futex word that is being watched by threads</param>
    public: static void PrivateFutexWakeAll(
      const volatile std::uint32_t &futexWord
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // NUCLEX_SUPPORT_INTEROP_LINUXFUTEXAPI_H
