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

#ifndef NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H
#define NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include <string> // std::string
#include <cstdint> // std::uint8_t and std::size_t

#include <ctime> // for ::timespec

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Linux futex synchronization API</summary>
  class LinuxFutexApi {

    // These are all for "private" futexes. Which means we hint to the Linux kernel
    // that the futex is private to the calling process (i.e. not in shared memory)
    // and certain assumptions and optimizations for that special case can be made.

    /// <summary>Waits for a private futex variable to change its value</summary>
    /// <param name="futexWord">Futex word that will be watched for changed</param>
    /// <param name="comparisonValue">
    ///   Value the futex word is expected to have, the method will return immediately
    ///   when the watched futex word has a different value.
    /// </param>
    /// <returns>
    ///   True if the comparison value has likely changed, false if the ait
    ///   was interrupted
    /// </returns>
    public: static bool PrivateFutexWait(
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
    /// <param name="timeoutFlagOutput">
    ///   Is set to true if the wait was cancelled due to reaching its timeout
    /// </param>
    /// <returns>
    ///   True if the comparison value has likely changed, false if the ait
    ///   was interrupted
    /// </returns>
    public: static bool PrivateFutexWait(
      const volatile std::uint32_t &futexWord,
      std::uint32_t comparisonValue,
      const ::timespec &patience,
      bool &timeoutFlagOutput
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

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H
