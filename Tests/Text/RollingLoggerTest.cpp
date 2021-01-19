#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Text/RollingLogger.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, RollingLoggerCanBeDefaultConstructed) {
    EXPECT_NO_THROW(
      RollingLogger logger;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, IsLoggingReturnsTrue) {
    Logger logger;
    EXPECT_TRUE(logger.IsLogging());

    // Negative test
    EXPECT_FALSE(Logger::Null.IsLogging());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerAcceptsPlainLogEntries) {
    RollingLogger logger;
    EXPECT_NO_THROW(
      logger.Inform(u8"This is a harmless message providing information");
      logger.Warn(u8"This is a warning indicating something is not optimal");
      logger.Complain(u8"This is an error and some action has failed completely");
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryCanBeExtracted) {
    RollingLogger logger;

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 0);

    logger.Inform(u8"This is a harmless message providing information");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 1);
    EXPECT_TRUE(history[0].find(u8"This is a harmless message") != std::string::npos);

    logger.Warn(u8"This is a warning indicating something is not optimal");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 2);
    EXPECT_TRUE(history[1].find(u8"This is a warning") != std::string::npos);

    logger.Complain(u8"This is an error and some action has failed completely");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 3);
    EXPECT_TRUE(history[2].find(u8"This is an error") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryKeepsMostRecentLines) {
    RollingLogger logger(2U); // 2 lines history length

    logger.Inform(u8"First line");
    logger.Inform(u8"Second line");
    logger.Inform(u8"Third line");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 2);
    EXPECT_TRUE(history[0].find(u8"Second line") != std::string::npos);
    EXPECT_TRUE(history[1].find(u8"Third line") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryCanBeCleared) {
    RollingLogger logger;

    logger.Inform(u8"Test");
    logger.Inform(u8"Test");
    logger.Clear();
    logger.Inform(u8"First line");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1);
    EXPECT_TRUE(history[0].find(u8"First line") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendIntegers) {
    RollingLogger logger;

    logger.Append(12345);
    logger.Append(u8"Hello");
    logger.Append(54321);
    logger.Inform(u8"World");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1);
    EXPECT_TRUE(history[0].find(u8"12345Hello54321World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendFloatingPointValues) {
    RollingLogger logger;

    logger.Append(1.25f);
    logger.Append(u8"Hello");
    logger.Append(0.875);
    logger.Inform(u8"World");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1);
    EXPECT_TRUE(history[0].find(u8"1.25Hello0.875World") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogLinesCanBeIndented) {
    RollingLogger logger;

    logger.Inform(u8"Saving configuration {");
    {
      Logger::IndentationScope configurationLogScope(logger);

      logger.Append(u8"ResolutionX = ");
      logger.Append(1920);
      logger.Inform(std::string());

      logger.Append(u8"ResolutionY = ");
      logger.Append(1080);
      logger.Inform(std::string());
    }

    logger.Inform(u8"}");

    std::vector<std::string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 4);

    std::string::size_type logTextStartColumn = history[0].find(u8"Saving configuration {");
    ASSERT_NE(logTextStartColumn, std::string::npos);

    EXPECT_EQ(history[0].at(logTextStartColumn + 0), 'S'); // ...aving Configuration

    EXPECT_EQ(history[1].at(logTextStartColumn + 0), ' ');
    EXPECT_EQ(history[1].at(logTextStartColumn + 1), ' ');
    EXPECT_EQ(history[1].at(logTextStartColumn + 2), 'R'); // ...esolutionX

    EXPECT_EQ(history[2].at(logTextStartColumn + 0), ' ');
    EXPECT_EQ(history[2].at(logTextStartColumn + 1), ' ');
    EXPECT_EQ(history[2].at(logTextStartColumn + 2), 'R'); // ...resolutionY

    EXPECT_EQ(history[3].at(logTextStartColumn + 0), '}');
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
