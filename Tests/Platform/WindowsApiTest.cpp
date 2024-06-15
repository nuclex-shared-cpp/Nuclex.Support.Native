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

#include "../../Source/Platform/WindowsApi.h"

#include <gtest/gtest.h>

#if defined(NUCLEX_SUPPORT_WINDOWS)

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsApiTest, CanGetPosixErrorMessage) {
    int errorNumber = EACCES;
    std::string errorMessage = WindowsApi::GetErrorMessage(errorNumber);
    EXPECT_GT(errorMessage.length(), 10); // We can expect 10 letters at least, eh?
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsApiTest, CanGetWindowsErrorMessage) {
    DWORD errorCode = ERROR_OUTOFMEMORY;
    std::string errorMessage = WindowsApi::GetErrorMessage(errorCode);
    EXPECT_GT(errorMessage.length(), 10);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsApiTest, CanGetComErrorMessage) {
    HRESULT resultHandle = E_NOINTERFACE;
    std::string errorMessage = WindowsApi::GetErrorMessage(resultHandle);
    EXPECT_GT(errorMessage.length(), 10);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
