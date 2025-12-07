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

#include "../../Source/Interop/LinuxFileApi.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "Nuclex/Support/TemporaryFileScope.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()

#include <gtest/gtest.h>

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanOpenFileForReading) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    int fileDescriptor = LinuxFileApi::OpenFileForReading(tempFile.GetPath().c_str());
    ASSERT_NE(fileDescriptor, -1);

    ::close(fileDescriptor);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanOpenFileForWriting) {
    TemporaryFileScope tempFile;

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::uint8_t data[] = { 123 };
      ::ssize_t writtenByteCount = ::write(fileDescriptor, data, 1);
      ASSERT_EQ(writtenByteCount, 1);
    }

    struct ::stat fileStatus;
    int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(errorNumber, 0);
    EXPECT_EQ(fileStatus.st_size, 1);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanReadFromFile) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    {
      int fileDescriptor = LinuxFileApi::OpenFileForReading(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::byte buffer[11];
      std::size_t readByteCount = LinuxFileApi::Read(fileDescriptor, buffer, 11);
      ASSERT_EQ(readByteCount, 11U);

      EXPECT_EQ(buffer[0], static_cast<std::byte>(u8'H'));
      EXPECT_EQ(buffer[1], static_cast<std::byte>(u8'e'));
      EXPECT_EQ(buffer[2], static_cast<std::byte>(u8'l'));
      EXPECT_EQ(buffer[3], static_cast<std::byte>(u8'l'));
      EXPECT_EQ(buffer[4], static_cast<std::byte>(u8'o'));
      EXPECT_EQ(buffer[5], static_cast<std::byte>(u8' '));
      EXPECT_EQ(buffer[6], static_cast<std::byte>(u8'W'));
      EXPECT_EQ(buffer[7], static_cast<std::byte>(u8'o'));
      EXPECT_EQ(buffer[8], static_cast<std::byte>(u8'r'));
      EXPECT_EQ(buffer[9], static_cast<std::byte>(u8'l'));
      EXPECT_EQ(buffer[10], static_cast<std::byte>(u8'd'));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanWriteToFile) {
    TemporaryFileScope tempFile;

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::byte data[] = {
        static_cast<std::byte>(1),
        static_cast<std::byte>(2),
        static_cast<std::byte>(3),
        static_cast<std::byte>(4),
        static_cast<std::byte>(5)
      };
      LinuxFileApi::Write(fileDescriptor, data, 5);
    }

    struct ::stat fileStatus;
    int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(errorNumber, 0);
    EXPECT_EQ(fileStatus.st_size, 5);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, FileCanBeTruncated) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      struct ::stat fileStatus;
      int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
      ASSERT_EQ(errorNumber, 0);
      EXPECT_EQ(fileStatus.st_size, 11);

      LinuxFileApi::SetLength(fileDescriptor, 5);

      errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
      ASSERT_EQ(errorNumber, 0);
      EXPECT_EQ(fileStatus.st_size, 5);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // defined(NUCLEX_SUPPORT_LINUX)
