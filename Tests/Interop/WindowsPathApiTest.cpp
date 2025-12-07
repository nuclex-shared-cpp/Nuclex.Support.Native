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

#include "../../Source/Interop/WindowsPathApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, DetectsIfPathIsRelative) {
    EXPECT_TRUE(WindowsPathApi::IsPathRelative(L"Relative\\Path.txt"));
    EXPECT_TRUE(WindowsPathApi::IsPathRelative(L"R:elative\\Path.txt"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"\\Absolute\\Path"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"A:\\bsolute\\Path"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"\\\\UNC\\Path"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanAppendPath) {
    std::wstring testPath = L"C:\\Users";

    WindowsPathApi::AppendPath(testPath, L"Guest");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest");

    testPath.push_back(L'\\');
    WindowsPathApi::AppendPath(testPath, L"Documents");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest\\Documents");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanRemoveFilenameFromPath) {
    std::wstring testPath = L"C:\\ProgramData\\RandomFile.txt";
    WindowsPathApi::RemoveFileFromPath(testPath);
    EXPECT_EQ(testPath, L"C:\\ProgramData\\");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanDetectFilenameExtensionPresence) {
    EXPECT_TRUE(WindowsPathApi::HasExtension(L"C:\\TestFile.txt"));
    EXPECT_FALSE(WindowsPathApi::HasExtension(L"C:\\TestFile"));
    EXPECT_TRUE(WindowsPathApi::HasExtension(L"C:\\Directory.dir\\TestFile.txt"));
    EXPECT_FALSE(WindowsPathApi::HasExtension(L"C:\\Directory.dir\\TestFile"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanCheckIfFileExists) {
    std::wstring explorerPath;
    WindowsPathApi::GetWindowsDirectory(explorerPath);
    WindowsPathApi::AppendPath(explorerPath, L"explorer.exe");
    EXPECT_TRUE(WindowsPathApi::DoesFileExist(explorerPath));

    EXPECT_FALSE(WindowsPathApi::DoesFileExist(L"C:\\This\\Does\\Not\\Exist"));
    EXPECT_FALSE(WindowsPathApi::DoesFileExist(L"C:\\ThisDoesNotExist.txt"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanLocateWindowsDirectory) {
    std::wstring testPath;
    WindowsPathApi::GetWindowsDirectory(testPath);

    EXPECT_GE(testPath.length(), 4); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanLocateSystemDirectory) {
    std::wstring testPath;
    WindowsPathApi::GetSystemDirectory(testPath);

    EXPECT_GE(testPath.length(), 6); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
