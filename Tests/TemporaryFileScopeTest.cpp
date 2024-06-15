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

#include "Nuclex/Support/TemporaryFileScope.h"
#include <gtest/gtest.h>

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "../Source/Platform/WindowsApi.h"
#include "Nuclex/Support/Text/StringConverter.h"
#else
#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TemporaryFileScope scope(u8"tst");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, CreatesTemporaryFile) {
    TemporaryFileScope scope(u8"tst");

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(scope.GetPath());
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    int result = ::access(scope.GetPath().c_str(), R_OK);
    EXPECT_EQ(result, 0);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, TemporaryFileIsDeletedOnDestruction) {
    std::string path;
    {
      TemporaryFileScope scope(u8"tst");
      path = scope.GetPath();
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(path);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);
#else
    int result = ::access(path.c_str(), R_OK);
    EXPECT_LT(result, 0); // should be -1 for error
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, CanWriteStringToTemporaryFile) {
    TemporaryFileScope scope(u8"tst");

    scope.SetFileContents(std::string(u8"Hello World"));

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(scope.GetPath());
    ::WIN32_FILE_ATTRIBUTE_DATA fileInformation;
    BOOL result = GetFileAttributesExW(
      utf16Path.c_str(), GetFileExInfoStandard, &fileInformation
    );
    ASSERT_NE(result, FALSE);
    EXPECT_EQ(fileInformation.nFileSizeLow, 11U);
#else
    struct ::stat fileStatus;
    int result = ::stat(scope.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fileStatus.st_size, 11);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, CanWriteVectorToTemporaryFile) {
    TemporaryFileScope scope(u8"tst");

    std::vector<std::uint8_t> contents = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 };
    scope.SetFileContents(contents);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(scope.GetPath());
    ::WIN32_FILE_ATTRIBUTE_DATA fileInformation;
    BOOL result = GetFileAttributesExW(
      utf16Path.c_str(), GetFileExInfoStandard, &fileInformation
    );
    ASSERT_NE(result, FALSE);
    EXPECT_EQ(fileInformation.nFileSizeLow, 9U);
#else
    struct ::stat fileStatus;
    int result = ::stat(scope.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fileStatus.st_size, 9);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryFileScopeTest, WritingTwiceCanTruncateTemporaryFile) {
    TemporaryFileScope scope(u8"tst");

    scope.SetFileContents(std::string(u8"This is a long string that's written to the file"));
    scope.SetFileContents(std::string(u8"This one is short"));

#if defined(NUCLEX_SUPPORT_WINDOWS)
    struct ::stat fileStatus;
    int result = ::stat(scope.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fileStatus.st_size, 17);
#else
    struct ::stat fileStatus;
    int result = ::stat(scope.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fileStatus.st_size, 17);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
