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

#include "../../Source/Interop/PosixPathApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

namespace Nuclex::Support::Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixPathApiTest, DetectsIfPathIsRelative) {
    EXPECT_TRUE(PosixPathApi::IsPathRelative(u8"relative/path"));
    EXPECT_TRUE(PosixPathApi::IsPathRelative(u8"~file"));
    EXPECT_FALSE(PosixPathApi::IsPathRelative(u8"/absolute/path"));
    EXPECT_FALSE(PosixPathApi::IsPathRelative(u8"~/file"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixPathApiTest, CanAppendPath) {
    std::u8string testPath = u8"/home";

    PosixPathApi::AppendPath(testPath, u8"nobody");
    EXPECT_EQ(testPath, std::u8string(u8"/home/nobody"));

    testPath.push_back(u8'/');
    PosixPathApi::AppendPath(testPath, u8".bashrc");
    EXPECT_EQ(testPath, std::u8string(u8"/home/nobody/.bashrc"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixPathApiTest, CanRemoveFilenameFromPath) {
    std::u8string testPath = u8"/home/nobody/random-file";
    PosixPathApi::RemoveFileFromPath(testPath);
    EXPECT_EQ(testPath, std::u8string(u8"/home/nobody/"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixPathApiTest, CanCheckIfFileExists) {
    EXPECT_TRUE(PosixPathApi::DoesFileExist(u8"/proc/version"));

    EXPECT_FALSE(PosixPathApi::DoesFileExist(u8"/testing/this/does/not/exist"));
    EXPECT_FALSE(PosixPathApi::DoesFileExist(u8"/testing-this-does-not-exist"));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
