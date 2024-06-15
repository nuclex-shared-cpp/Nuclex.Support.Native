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

#include "Nuclex/Support/Threading/Process.h"

#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "../Source/Platform/WindowsApi.h"
#include "Nuclex/Support/Text/StringConverter.h"
#else
#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()
#endif

#include <stdexcept> // for std::logic_error

// An executable that is in the default search path, has an exit code of 0,
// does not need super user privileges and does nothing bad when run.
#if defined(NUCLEX_SUPPORT_WINDOWS)
  #define NUCLEX_SUPPORT_HARMLESS_EXECUTABLE u8"hostname.exe"
#else
  #define NUCLEX_SUPPORT_HARMLESS_EXECUTABLE u8"ls"
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simple observe that captures and collects a process' output stream</summary>
  class Observer {

    /// <summary>Collects output sent to stdout</summary>
    /// <param name="characters">Buffer containing the characters sent to stdout</param>
    /// <param name="count">Number of characters that have been sent to stdout</param>
    public: void AcceptStdOut(const char *characters, std::size_t count) {
      this->output.append(characters, count);
    }

    /// <summary>String in which all output sent to stdout accumulates</summary>
    public: std::string output;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, UnstartedProcessIsNotRunning) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);
    EXPECT_FALSE(test.IsRunning());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, WaitingOnUnstartedProcessCausesException) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);
    EXPECT_THROW(
      test.Wait(),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, JoiningUnstartedProcessCausesException) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);
    EXPECT_THROW(
      test.Join(),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, ProcessCanBeStarted) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);

    test.Start();

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, JoinAfterWaitIsLegal) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);

    test.Start();
    EXPECT_TRUE(test.Wait());

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, WaitAfterJoinCausesException) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);

    test.Start();
    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);

    EXPECT_THROW(
      test.Wait(),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, DoubleJoinThrowsException) {
    Process test(NUCLEX_SUPPORT_HARMLESS_EXECUTABLE);

    test.Start();
    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);

    EXPECT_THROW(
      test.Join(),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, CanTellIfProcessIsStillRunning) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    // Sleep does not ship with all Windows 10 releases
    // Timeout immediately error-exits if stdin is redirected
    // Ping with existing IP always waits 1 second between pings            <--- the only option
    // Ping with non-existing IP waits until timeout, but returns exit code 1
    Process test(u8"ping");
    test.Start({u8"-n 2", u8"-4 127.0.0.1"});
#else
    Process test(u8"sleep");
    test.Start({u8"0.25"});
#endif

    EXPECT_TRUE(test.IsRunning());
    EXPECT_TRUE(test.IsRunning());

    //test.Kill(std::chrono::milliseconds(10)); // test it! :)
    EXPECT_TRUE(test.Wait());

    EXPECT_FALSE(test.IsRunning());
    EXPECT_FALSE(test.IsRunning());

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);

    EXPECT_FALSE(test.IsRunning());
    EXPECT_FALSE(test.IsRunning());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, CanCaptureStdout) {
    Observer observer;

#if defined(NUCLEX_SUPPORT_WINDOWS)
    Process test(u8"cmd.exe");
    test.StdOut.Subscribe<Observer, &Observer::AcceptStdOut>(&observer);
    test.Start({ u8"/c", "dir", "/b" });
#else
    Process test(u8"ls");
    test.StdOut.Subscribe<Observer, &Observer::AcceptStdOut>(&observer);
    test.Start({ u8"-l" });
#endif

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);

    // Check for some directories that should have been listed by ls / dir
    EXPECT_GE(observer.output.length(), 21U);
    //EXPECT_NE(observer.output.find(u8".."), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, ProvidesPathOfRunningExecutable) {
    std::string executableDirectory = Process::GetExecutableDirectory();

#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring executablePathUtf16 = Text::StringConverter::WideFromUtf8(
      executableDirectory + std::string(u8"Nuclex.Support.Native.Tests.exe")
    );

    ::WIN32_FILE_ATTRIBUTE_DATA fileInformation;
    BOOL result = GetFileAttributesExW(
      executablePathUtf16.c_str(), GetFileExInfoStandard, &fileInformation
    );
    ASSERT_NE(result, FALSE);
    EXPECT_GE(fileInformation.nFileSizeLow, 10000U); // We should be more than 10000 bytes long
#else
    std::string executablePath = (
      executableDirectory + std::string(u8"NuclexSupportNativeTests")
    );

    struct ::stat fileStatus;
    int result = ::stat(executablePath.c_str(), &fileStatus);
    ASSERT_EQ(result, 0);
    EXPECT_GE(fileStatus.st_size, 10000); // We should be more than 10000 bytes long
#endif
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  TEST(ProcessTest, ChildSegmentationFaultCausesExceptionInJoin) {
    Process test("./segfault");

    test.Start();
    EXPECT_THROW(
      test.Join(),
      std::runtime_error
    );
  }
#endif // defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  TEST(ProcessTest, ExitCodeIsCapturedByJoin) {
    Process test("./badexit");

    test.Start();
    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 1);
  }
#endif // defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  TEST(ProcessTest, ExitCodeIsCapturedByWait) {
    Process test("./badexit");

    test.Start();
    test.Wait(); // Wait reaps the zombie process here on Linux systems
    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 1);
  }
#endif // defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  TEST(ProcessTest, ExitCodeIsCapturedByisRunning) {
    Process test("./badexit");

    test.Start();
    while(test.IsRunning()) {
      ;
    }
    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 1);
  }
#endif // defined(NUCLEX_SUPPORT_HAVE_TEST_EXECUTABLES)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)
