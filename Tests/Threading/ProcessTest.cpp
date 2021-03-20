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

#include "Nuclex/Support/Threading/Process.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, InstancesCanBeCreated) {
#if defined(NUCLEX_SUPPORT_WIN32)
    Process test("net.exe");
#else
    Process test("ls");
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, ProcessCanBeStarted) {
#if defined(NUCLEX_SUPPORT_WIN32)
    Process test("net.exe");
#else
    Process test("ls");
#endif

    test.Start();

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ProcessTest, CanTellIfProcessIsStillRunning) {
#if defined(NUCLEX_SUPPORT_WIN32)
    Process test(u8"cmd.exe");
    test.Start({u8"/c sleep 1"});
#else
    Process test(u8"sleep");
    test.Start({u8"0.25"});
#endif

    EXPECT_TRUE(test.IsRunning());
    EXPECT_TRUE(test.IsRunning());

    int exitCode = test.Join();
    EXPECT_EQ(exitCode, 0);

    EXPECT_FALSE(test.IsRunning());
    EXPECT_FALSE(test.IsRunning());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
