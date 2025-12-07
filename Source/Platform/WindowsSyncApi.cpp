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

#include "WindowsSyncApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/StringConverter.h"

#include <cassert> // for assert()
#include <synchapi.h> // for ::WaitOnAddress()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  WindowsSyncApi::WaitResult WindowsSyncApi::waitOnAddressWithTimeout(
    const volatile void *waitVariableAddress,
    void *comparisonValue,
    std::size_t waitVariableByteCount,
    std::chrono::milliseconds patience
  ) {
    BOOL result = ::WaitOnAddress(
      const_cast<volatile VOID *>(waitVariableAddress),
      reinterpret_cast<PVOID>(comparisonValue),
      static_cast<SIZE_T>(waitVariableByteCount),
      static_cast<DWORD>(patience.count())
    );
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      if(errorCode == ERROR_TIMEOUT) [[likely]] {
        return WaitResult::TimedOut;
      }

      WindowsApi::ThrowExceptionForSystemError(u8"Could not wait on memory address", errorCode);
    }

    return WaitResult::ValueChanged;
  }

  // ------------------------------------------------------------------------------------------- //

  WindowsSyncApi::WaitResult WindowsSyncApi::waitOnAddressNoTimeout(
    const volatile void *waitVariableAddress,
    void *comparisonValue,
    std::size_t waitVariableByteCount
  ) {
    BOOL result = ::WaitOnAddress(
      const_cast<volatile VOID *>(waitVariableAddress),
      reinterpret_cast<PVOID>(comparisonValue),
      static_cast<SIZE_T>(waitVariableByteCount),
      INFINITE
    );
    if(result == FALSE) [[unlikely]] {
      DWORD errorCode = ::GetLastError();
      WindowsApi::ThrowExceptionForSystemError(u8"Could not wait on memory address", errorCode);
    }

    return WaitResult::ValueChanged;
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsSyncApi::wakeByAddressAll(const volatile void *waitVariableAddress) {
    ::WakeByAddressAll(const_cast<PVOID>(waitVariableAddress));
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsSyncApi::wakeByAddressSingle(const volatile void *waitVariableAddress) {
    ::WakeByAddressSingle(const_cast<PVOID>(waitVariableAddress));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
