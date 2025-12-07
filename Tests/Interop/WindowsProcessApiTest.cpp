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

#include "../../Source/Interop/WindowsProcessApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "../Source/Interop/WindowsPathApi.h"

#include <gtest/gtest.h>

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInWindowsDirectory) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.exe");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, CustomExtensionisRespected) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.exe");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.x");

      EXPECT_EQ(path, L"notepad.x");
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInSystemDirectory) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"ping.exe");

      EXPECT_GT(path.length(), 13); // shortest possible valid path
      EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"ping");

      EXPECT_GT(path.length(), 13); // shortest possible valid path
      EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInOwnDirectory) {
    std::wstring path;
    WindowsProcessApi::GetAbsoluteExecutablePath(path, L"Nuclex.Support.Native.Tests.exe");

    EXPECT_GT(path.length(), 35); // shortest possible valid path
    EXPECT_TRUE(WindowsPathApi::DoesFileExist(path));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, RelativeWorkingDirectoryStartsInOwnDirectory) {
    std::wstring path;
    WindowsProcessApi::GetAbsoluteExecutablePath(path, L"Nuclex.Support.Native.Tests.exe");

    std::wstring directory;
    WindowsProcessApi::GetAbsoluteWorkingDirectory(directory, L".");

    // The directory may end with a \\. since we specified '.' as the target.
    // This isn't required, so we accept both variants. In case the dot is returned,
    // remove it so the path can be compared against the executable path.
    if(directory.length() >= 2) {
      if(directory[directory.length() - 1] == L'.') {
        if(directory[directory.length() - 2] == L'\\') {
          directory.resize(directory.length() - 2);
        } else {
          directory.resize(directory.length() - 1);
        }
      }
    }

    EXPECT_GT(directory.length(), 4); // shortest possible valid path
    EXPECT_NE(path.find(directory), std::wstring::npos);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platfor

#endif // defined(NUCLEX_SUPPORT_WINDOWS)