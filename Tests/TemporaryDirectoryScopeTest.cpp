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
#include "../Source/Platform/WindowsApi.h" // for WindowsApi
#include "../Source/Platform/WindowsFileApi.h" // for WindowsApi
#include "Nuclex/Support/Text/StringConverter.h" // for UTF-8 to wide char conversion
#else
#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()
#include <sys/types.h> // for S_ISDIR
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TemporaryDirectoryScope scope(u8"tst");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CreatesTemporaryDirectory) {
    TemporaryDirectoryScope scope(u8"tst");

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::string path = scope.GetPath();
    if(path.length() > 0) {
      char current = path[path.length() - 1];
      if((current == '/') || (current == '\\')) {
        path.erase(path.begin() + (path.length() - 1));
      }
    }

    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(path);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    std::string path = scope.GetPath();
    if(path.length() > 0) {
      if(path[path.length() - 1] == '/') {
        path.erase(path.begin() + (path.length() - 1));
      }
    }

    struct ::stat fileStatus;
    int result = ::stat(path.c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(S_ISDIR(fileStatus.st_mode));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, TemporaryFileIsDeletedOnDestruction) {
    std::string path;
    {
      TemporaryDirectoryScope scope(u8"tst");

      path = scope.GetPath();
      if(path.length() > 0) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
        char current = path[path.length() - 1];
        if((current == '/') || (current == '\\')) {
          path.erase(path.begin() + (path.length() - 1));
        }
#else
        if(path[path.length() - 1] == '/') {
          path.erase(path.begin() + (path.length() - 1));
        }
#endif
      }
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(path);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);
#else
    struct ::stat fileStatus;
    int result = ::stat(path.c_str(), &fileStatus);
    EXPECT_LT(result, 0); // the directory should not exist anymore in any form
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanCreateFilesFromStrings) {
    TemporaryDirectoryScope scope(u8"tst");

    std::string firstFilePath = scope.PlaceFile(u8"first", std::string(u8"First file."));
    std::string secondFilePath = scope.PlaceFile(u8"second", std::string(u8"Second file."));

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(firstFilePath);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = Text::StringConverter::WideFromUtf8(secondFilePath);
    attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    int result = ::access(firstFilePath.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    result = ::access(secondFilePath.c_str(), R_OK);
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

    std::string firstFilePath = scope.PlaceFile(u8"first", firstContents);
    std::string secondFilePath = scope.PlaceFile(u8"second", secondContents);

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(firstFilePath);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = Text::StringConverter::WideFromUtf8(secondFilePath);
    attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_NE(attributes, INVALID_FILE_ATTRIBUTES);
#else
    int result = ::access(firstFilePath.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    result = ::access(secondFilePath.c_str(), R_OK);
    EXPECT_EQ(result, 0);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanReadFilesIntoStrings) {
    TemporaryDirectoryScope scope(u8"tst");

    scope.PlaceFile(u8"first", std::string(u8"First file."));
    scope.PlaceFile(u8"second", std::string(u8"Second file."));

    std::string contents1, contents2;
    scope.ReadFile(u8"second", contents2);
    scope.ReadFile(u8"first", contents1);

    ASSERT_EQ(contents1, std::string(u8"First file."));
    ASSERT_EQ(contents2, std::string(u8"Second file."));
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
    std::string firstFilePath, secondFilePath;
    {
      TemporaryDirectoryScope scope(u8"tst");

      firstFilePath = scope.PlaceFile(u8"a.txt", std::string(u8"First file."));
      secondFilePath = scope.PlaceFile(u8"b.txt", std::string(u8"Second file."));
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(firstFilePath);
    DWORD attributes = ::GetFileAttributesW(utf16Path.c_str());
    EXPECT_EQ(attributes, INVALID_FILE_ATTRIBUTES);

    utf16Path = Text::StringConverter::WideFromUtf8(secondFilePath);
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

}} // namespace Nuclex::Support
