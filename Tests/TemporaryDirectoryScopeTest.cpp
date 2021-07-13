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

#include "Nuclex/Support/TemporaryDirectoryScope.h"
#include <gtest/gtest.h>

#if !defined(NUCLEX_SUPPORT_WINDOWS)
#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()
#include <sys/types.h> // for S_ISDIR
#else
#include "../Source/Platform/WindowsApi.h"
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TemporaryDirectoryScope scope;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CreatesTemporaryDirectory) {
    TemporaryDirectoryScope scope;

#if !defined(NUCLEX_SUPPORT_WINDOWS)
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
#else
    // TODO
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, TemporaryFileIsDeletedOnDestruction) {
    std::string path;
    {
      TemporaryDirectoryScope scope;

      path = scope.GetPath();
      if(path.length() > 0) {
        if(path[path.length() - 1] == '/') {
          path.erase(path.begin() + (path.length() - 1));
        }
      }
    }
    
#if !defined(NUCLEX_SUPPORT_WINDOWS)
    struct ::stat fileStatus;
    int result = ::stat(path.c_str(), &fileStatus);
    EXPECT_LT(result, 0); // the directory should not exist anymore in any form
#else
    // TODO
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanCreateFilesFromStrings) {
    TemporaryDirectoryScope scope;

    std::string firstFile = scope.PlaceFile(u8"first", std::string(u8"First file."));
    std::string secondFile = scope.PlaceFile(u8"second", std::string(u8"Second file."));

    int result = ::access(firstFile.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    result = ::access(secondFile.c_str(), R_OK);
    EXPECT_EQ(result, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, CanCreateFilesFromVectors) {
    TemporaryDirectoryScope scope;

    std::vector<std::uint8_t> firstContents = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
    std::vector<std::uint8_t> secondContents = { 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1 };

    std::string firstFile = scope.PlaceFile(u8"first", firstContents);
    std::string secondFile = scope.PlaceFile(u8"second", secondContents);

    int result = ::access(firstFile.c_str(), R_OK);
    EXPECT_EQ(result, 0);

    result = ::access(secondFile.c_str(), R_OK);
    EXPECT_EQ(result, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(TemporaryDirectoryScopeTest, FilesGetDeletedWithTemporaryDirectory) {
    std::string firstFilePath, secondFilePath;
    {
      TemporaryDirectoryScope scope;

      firstFilePath = scope.PlaceFile(u8"a.txt", std::string(u8"First file."));
      secondFilePath = scope.PlaceFile(u8"b.txt", std::string(u8"Second file."));
    }

    int result = ::access(firstFilePath.c_str(), R_OK);
    EXPECT_LT(result, 0); // should be -1 for failure

    result = ::access(secondFilePath.c_str(), R_OK);
    EXPECT_LT(result, 0); // should be -1 for failure
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
