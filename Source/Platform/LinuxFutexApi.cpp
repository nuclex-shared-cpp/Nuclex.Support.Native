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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "LinuxFutexApi.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "PosixApi.h" // Linux uses Posix error handling

#include <cerrno> // To access ::errno directly

#include <linux/futex.h> // for futex constants
#include <unistd.h> // for ::syscall()
#include <limits.h> // for INT_MAX
#include <sys/syscall.h> // for ::SYS_futex

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  LinuxFutexApi::WaitResult LinuxFutexApi::PrivateFutexWait(
    const volatile std::uint32_t& futexWord,
    std::uint32_t comparisonValue
  ) {

    // Futex Wait (Linux 2.6.0+)
    // https://man7.org/linux/man-pages/man2/futex.2.html
    //
    // This sends the thread to sleep for as long as the futex word has the expected value.
    // Checking and entering sleep is one atomic operation, avoiding a race condition.
    long result = ::syscall(
      SYS_futex, // syscall id
      static_cast<const volatile std::uint32_t *>(&futexWord), // futex word being accessed
      static_cast<int>(FUTEX_PRIVATE_FLAG | FUTEX_WAIT), // process-private futex wakeup
      static_cast<int>(comparisonValue), // wait while futex word is equal to this value
      static_cast<struct ::timespec *>(nullptr), // timeout -> infinite
      static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
      static_cast<int>(0) // second futex word value -> ignored
    );
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      if(errorNumber == EAGAIN) [[likely]] { // Value was not 0, so gate is now open
        return WaitResult::ValueChanged;
      } else if(errorNumber != EINTR) {
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not sleep via futex wait. Ancient Linux kernel version?", errorNumber
        );
      }
    }

    // According to the man pages, FUTEX_WAIT returns 0 if the caller was woken up,
    // in which case we return true as well (the other case is when we get an errno
    // of EAGAIN, indicating that the futex word has changed its value.
    return (result == 0) ? WaitResult::ValueChanged : WaitResult::Interrupted;

  }

  // ------------------------------------------------------------------------------------------- //

  LinuxFutexApi::WaitResult LinuxFutexApi::PrivateFutexWait(
    const volatile std::uint32_t& futexWord,
    std::uint32_t comparisonValue,
    const ::timespec &patience
  ) {

    // Futex Wait (Linux 2.6.0+)
    // https://man7.org/linux/man-pages/man2/futex.2.html
    //
    // This sends the thread to sleep for as long as the futex word has the expected value.
    // Checking and entering sleep is one atomic operation, avoiding a race condition.
    long result = ::syscall(
      SYS_futex, // syscall id
      static_cast<const volatile std::uint32_t *>(&futexWord), // futex word being accessed
      static_cast<int>(FUTEX_PRIVATE_FLAG | FUTEX_WAIT), // process-private futex wakeup
      static_cast<int>(comparisonValue), // wait while futex word is equal to this value
      static_cast<const struct ::timespec *>(&patience), // timeout after which to fail
      static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
      static_cast<int>(0) // second futex word value -> ignored
    );
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      if(errorNumber == EAGAIN) [[likely]] { // Value was not 0, so gate is now open
        return WaitResult::ValueChanged;
      } else if(errorNumber == ETIMEDOUT) [[likely]] { // Timeout, wait failed
        return WaitResult::TimedOut;
      } else if(errorNumber != EINTR) {
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not sleep via futex wait. Ancient Linux kernel version?", errorNumber
        );
      }
    }

    // According to the man pages, FUTEX_WAIT returns 0 if the caller was woken up,
    // in which case we return true as well (the other case is when we get an errno
    // of EAGAIN, indicating that the futex word has changed its value.
    return (result == 0) ? WaitResult::ValueChanged : WaitResult::Interrupted;

  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFutexApi::PrivateFutexWakeSingle(const volatile std::uint32_t &futexWord) {

    // Futex Wake (Linux 2.6.0+)
    // https://man7.org/linux/man-pages/man2/futex.2.html
    //
    // This will signal other threads sitting in the PrivateFutexWait() method to
    // re-check their futex word and resume running
    long result = ::syscall(
      SYS_futex, // syscall id
      static_cast<const volatile std::uint32_t *>(&futexWord), // futex word being accessed
      static_cast<int>(FUTEX_PRIVATE_FLAG | FUTEX_WAKE), // process-private futex wakeup
      static_cast<int>(1), // wake up an waiter
      static_cast<struct ::timespec *>(nullptr), // timeout -> ignored
      static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
      static_cast<int>(0) // second futex word value -> ignored
    );
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not wake up thread waiting on futex", errorNumber
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFutexApi::PrivateFutexWakeAll(const volatile std::uint32_t &futexWord) {

    // Futex Wake (Linux 2.6.0+)
    // https://man7.org/linux/man-pages/man2/futex.2.html
    //
    // This will signal other threads sitting in the PrivateFutexWait() method to
    // re-check their futex word and resume running
    long result = ::syscall(
      SYS_futex, // syscall id
      static_cast<const volatile std::uint32_t *>(&futexWord), // futex word being accessed
      static_cast<int>(FUTEX_PRIVATE_FLAG | FUTEX_WAKE), // process-private futex wakeup
      static_cast<int>(INT_MAX), // wake up all waiters
      static_cast<struct ::timespec *>(nullptr), // timeout -> ignored
      static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
      static_cast<int>(0) // second futex word value -> ignored
    );
    if(result == -1) [[unlikely]] {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not wake up threads waiting on futex", errorNumber
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)
