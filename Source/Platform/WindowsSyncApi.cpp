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

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  bool WindowsSyncApi::waitOnAddressWithTimeout(
    const volatile void *waitVariableAddress,
    void *comparisonValue,
    std::size_t waitVariableByteCount,
    std::chrono::milliseconds maximumWaitTime
  ) {
    BOOL result = ::WaitOnAddress(
      const_cast<volatile VOID *>(waitVariableAddress),
      reinterpret_cast<PVOID>(comparisonValue),
      static_cast<SIZE_T>(waitVariableByteCount),
      static_cast<DWORD>(maximumWaitTime.count())
    );
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      if(likely(errorCode == ERROR_TIMEOUT)) {
        return false;
      }

      WindowsApi::ThrowExceptionForSystemError(u8"Could not wait on memory address", errorCode);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsSyncApi::waitOnAddressNoTimeout(
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
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      WindowsApi::ThrowExceptionForSystemError(u8"Could not wait on memory address", errorCode);
    }

    return true;
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

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
