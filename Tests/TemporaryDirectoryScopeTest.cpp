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

#include "Nuclex/Support/TemporaryDirectoryScope.h"
#include <gtest/gtest.h>

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "../Source/Interop/WindowsApi.h" // for WindowsApi
#include "../Source/Interop/WindowsFileApi.h" // for WindowsApi
#include "Nuclex/Support/Text/StringConverter.h" // for UTF-8 to wide char conversion
#else
#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()
#include <sys/types.h> // for S_ISDIR
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Removes the final character from a path if it is a directory separator</summary>
  /// <param name="pathString">
  ///   Path string from which a trailing directory separator will be removed
  /// </param>
  void removeTrailingDirectorySeparator(std::u8string &pathString) {
    std::u8string::size_type length = pathString.length();
    if(0 < length) {
      char8_t lastCharacter = pathString[length -1];
#if defined(NUCLEX_SUPPORT_WINDOWS)
      if((lastCharacter == '/') || (lastCharacter == '\\')) {
#else
      if(lastCharacter == '/') {
#endif
        pathString.erase(pathString.begin() + (length - 1));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TemporaryDirectoryScope scope(u8"tst");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CreatesTemporaryDirectory) {
    TemporaryDirectoryScope scope(u8"tst");

    std::filesystem::path path = scope.GetPath();
    ASSERT_FALSE(path.empty());

    std::u8string pathString = path.u8string();
    removeTrailingDirectorySeparator(pathString);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(pathString);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    std::string pathStringChars(pathString.begin(), pathString.end());
    struct ::stat fileStatus;
    int result = ::stat(pathStringChars.c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(S_ISDIR(fileStatus.st_mode));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, TemporaryFileIsDeletedOnDestruction) {
    std::filesystem::path path;
    {
      TemporaryDirectoryScope scope(u8"tst");
      path = scope.GetPath();
    }

    std::u8string pathString = path.u8string();
    removeTrailingDirectorySeparator(pathString);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(pathString);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);
#else
    std::string pathStringChars(pathString.begin(), pathString.end());
    struct ::stat fileStatus;
    int result = ::stat(pathStringChars.c_str(), &fileStatus);
    EXPECT_LT(result, 0); // the directory should not exist anymore in any form
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanCreateFilesFromStrings) {
    TemporaryDirectoryScope scope(u8"tst");

    std::filesystem::path firstFilePath = scope.PlaceFile(
      u8"first", std::u8string(u8"First file.")
    );
    std::filesystem::path secondFilePath = scope.PlaceFile(
      u8"second", std::u8string(u8"Second file.")
    );

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = firstFilePath.wstring();
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = secondFilePath.wstring();
    attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    std::u8string firstFilePathString = firstFilePath.u8string();
    std::string firstFilePathChars(firstFilePathString.begin(), firstFilePathString.end());
    int result = ::access(firstFilePathChars.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    std::u8string secondFilePathString = secondFilePath.u8string();
    std::string secondFilePathChars(secondFilePathString.begin(), secondFilePathString.end());
    result = ::access(secondFilePathChars.c_str(), R_OK);
    EXPECT_EQ(result, 0);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanCreateFilesFromVectors) {
    TemporaryDirectoryScope scope(u8"tst");

    std::vector<std::byte> firstContents = {
      static_cast<std::byte>(0x1), static_cast<std::byte>(0x2),
      static_cast<std::byte>(0x3), static_cast<std::byte>(0x4),
      static_cast<std::byte>(0x5), static_cast<std::byte>(0x6),
      static_cast<std::byte>(0x7), static_cast<std::byte>(0x8)
    };
    std::vector<std::byte> secondContents = {
      static_cast<std::byte>(0x8), static_cast<std::byte>(0x7),
      static_cast<std::byte>(0x6), static_cast<std::byte>(0x5),
      static_cast<std::byte>(0x4), static_cast<std::byte>(0x3),
      static_cast<std::byte>(0x2), static_cast<std::byte>(0x1)
    };

    std::filesystem::path firstFilePath = scope.PlaceFile(u8"first", firstContents);
    std::filesystem::path secondFilePath = scope.PlaceFile(u8"second", secondContents);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = firstFilePath.wstring();
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = secondFilePath.wstring();
    attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    std::u8string firstFilePathString = firstFilePath.u8string();
    std::string firstFilePathChars(firstFilePathString.begin(), firstFilePathString.end());
    int result = ::access(firstFilePathChars.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    std::u8string secondFilePathString = secondFilePath.u8string();
    std::string secondFilePathChars(secondFilePathString.begin(), secondFilePathString.end());
    result = ::access(secondFilePathChars.c_str(), R_OK);
    EXPECT_EQ(result, 0);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanReadFilesIntoStrings) {
    TemporaryDirectoryScope scope(u8"tst");

    scope.PlaceFile(u8"first", std::u8string(u8"First file."));
    scope.PlaceFile(u8"second", std::u8string(u8"Second file."));

    std::u8string contents1, contents2;
    scope.ReadFile(u8"second", contents2);
    scope.ReadFile(u8"first", contents1);

    ASSERT_EQ(contents1, std::u8string(u8"First file."));
    ASSERT_EQ(contents2, std::u8string(u8"Second file."));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanReadFilesIntoVectors) {
    TemporaryDirectoryScope scope(u8"tst");

    std::vector<std::byte> contents = {
      static_cast<std::byte>(0x42), static_cast<std::byte>(0x43),
      static_cast<std::byte>(0x44), static_cast<std::byte>(0x45),
      static_cast<std::byte>(0x46), static_cast<std::byte>(0x47)
    };
    scope.PlaceFile(u8"this-is-a-test-file", contents);
    std::vector<std::byte> readBack = scope.ReadFile(u8"this-is-a-test-file");

    ASSERT_EQ(contents.size(), readBack.size());
    for(std::size_t index = 0; index < contents.size(); ++index) {
      EXPECT_EQ(contents[index], readBack[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, FilesGetDeletedWithTemporaryDirectory) {
    std::filesystem::path firstFilePath, secondFilePath;
    {
      TemporaryDirectoryScope scope(u8"tst");

      firstFilePath = scope.PlaceFile(u8"a.txt", std::u8string(u8"First file."));
      secondFilePath = scope.PlaceFile(u8"b.txt", std::u8string(u8"Second file."));
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = firstFilePath.wstring();
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = secondFilePath.wstring();
    attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);
#else
    int result = ::access(firstFilePath.c_str(), R_OK);
    EXPECT_LT(result, 0); // should be -1 for failure

    result = ::access(secondFilePath.c_str(), R_OK);
    EXPECT_LT(result, 0); // should be -1 for failure
#endif
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support
