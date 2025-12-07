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

#include "../../Source/Interop/PosixProcessApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "../Source/Interop/PosixPathApi.h"

#include <gtest/gtest.h>

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixProcessApiTest, ExecutableIsResolvedInUsrBinDirectory) {
    std::filesystem::path path;
    PosixProcessApi::GetAbsoluteExecutablePath(path, u8"ls");

    std::u8string pathString = path.u8string();
    EXPECT_GT(pathString.length(), 5U); // shortest possible valid path
    EXPECT_TRUE(PosixPathApi::DoesFileExist(path));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixProcessApiTest, ExecutableIsResolvedInOwnDirectory) {
    std::filesystem::path path;
    PosixProcessApi::GetAbsoluteExecutablePath(path, u8"NuclexSupportNativeTests");

    std::u8string pathString = path.u8string();
    EXPECT_GT(pathString.length(), 26U); // shortest possible valid path
    EXPECT_TRUE(PosixPathApi::DoesFileExist(path));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixProcessApiTest, RelativeWorkingDirectoryStartsInOwnDirectory) {
    std::filesystem::path path;
    PosixProcessApi::GetAbsoluteExecutablePath(path, u8"NuclexSupportNativeTests");

    std::filesystem::path directory;
    PosixProcessApi::GetAbsoluteWorkingDirectory(directory, u8".");

    // The directory may end with a /. since we specified '.' as the target.
    // This isn't required, so we accept both variants. In case the dot is returned,
    // remove it so the path can be compared against the executable path.
    std::u8string directoryString = directory.u8string();
    if(directoryString.length() >= 2) {
      if(directoryString[directoryString.length() - 1] == '.') {
        if(directoryString[directoryString.length() - 2] == '/') {
          directoryString.resize(directoryString.length() - 2);
        } else {
          directoryString.resize(directoryString.length() - 1);
        }
      }
    }

    EXPECT_GT(directoryString.length(), 2U); // shortest possible valid path
    std::u8string pathString = path.u8string();
    EXPECT_NE(pathString.find(directoryString), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
