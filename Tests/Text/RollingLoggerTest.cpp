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

#include "Nuclex/Support/Text/RollingLogger.h"

#include <gtest/gtest.h>

namespace Nuclex::Support::Text {

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

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 0U);

    logger.Inform(u8"This is a harmless message providing information");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_NE(history[0].find(u8"This is a harmless message"), std::u8string::npos);

    logger.Warn(u8"This is a warning indicating something is not optimal");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 2U);
    EXPECT_NE(history[1].find(u8"This is a warning"), std::u8string::npos);

    logger.Complain(u8"This is an error and some action has failed completely");
    history = logger.GetLines();
    EXPECT_EQ(history.size(), 3U);
    EXPECT_NE(history[2].find(u8"This is an error"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryKeepsMostRecentLines) {
    RollingLogger logger(2); // 2 lines history length

    logger.Inform(u8"First line");
    logger.Inform(u8"Second line");
    logger.Inform(u8"Third line");

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 2U);
    EXPECT_NE(history[0].find(u8"Second line"), std::u8string::npos);
    EXPECT_NE(history[1].find(u8"Third line"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogHistoryCanBeCleared) {
    RollingLogger logger;

    logger.Inform(u8"Test");
    logger.Inform(u8"Test");
    logger.Clear();
    logger.Inform(u8"First line");

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_NE(history[0].find(u8"First line"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendIntegers) {
    RollingLogger logger;

    logger.Append(12345);
    logger.Append(u8"Hello");
    logger.Append(54321);
    logger.Inform(u8"World");

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_NE(history[0].find(u8"12345Hello54321World"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LoggerCanAppendFloatingPointValues) {
    RollingLogger logger;

    logger.Append(1.25f);
    logger.Append(u8"Hello");
    logger.Append(0.875);
    logger.Inform(u8"World");

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 1U);
    EXPECT_NE(history[0].find(u8"1.25Hello0.875World"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, LogLinesCanBeIndented) {
    RollingLogger logger;

    logger.Inform(u8"Saving configuration {");
    {
      Logger::IndentationScope configurationLogScope(logger);

      logger.Append(u8"ResolutionX = ");
      logger.Append(1920);
      logger.Inform(std::u8string());

      logger.Append(u8"ResolutionY = ");
      logger.Append(1080);
      logger.Inform(std::u8string());
    }
    logger.Inform(u8"}");

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 4U);

    // Figure out which column log messages begin in
    std::u8string::size_type logTextStartColumn = history[0].find(u8"Saving configuration {");
    ASSERT_NE(logTextStartColumn, std::u8string::npos);

    // Check the indentation by looked at the first few characters of each line's message
    {
      EXPECT_EQ(history[0].at(logTextStartColumn + 0), u8'S'); // ...aving Configuration

      EXPECT_EQ(history[1].at(logTextStartColumn + 0), u8' ');
      EXPECT_EQ(history[1].at(logTextStartColumn + 1), u8' ');
      EXPECT_EQ(history[1].at(logTextStartColumn + 2), u8'R'); // ...esolutionX

      EXPECT_EQ(history[2].at(logTextStartColumn + 0), u8' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 1), u8' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 2), u8'R'); // ...resolutionY

      EXPECT_EQ(history[3].at(logTextStartColumn + 0), u8'}');
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RollingLoggerTest, IndendationAffectsLineBeingFormed) {
    RollingLogger logger;

    logger.Inform(u8"Not indented");

    // Start the line appended, but then stop indentation before finalizing it
    // The logger has to remove indentation before the text without destroying it
    {
      Logger::IndentationScope configurationLogScope(logger);
      logger.Append(12345);
    }
    logger.Warn(u8"Warning");

    // Start the line unindented, but then begin indentation before finalizing it
    // The logger has to insert indentation before the text without destroying it
    logger.Append(54321);
    {
      Logger::IndentationScope configurationLogScope(logger);
      logger.Complain(u8"Error");
    }

    std::vector<std::u8string> history = logger.GetLines();
    EXPECT_EQ(history.size(), 3U);

    // Figure out which column log messages begin in
    std::u8string::size_type logTextStartColumn = history[0].find(u8"Not indented");
    ASSERT_NE(logTextStartColumn, std::u8string::npos);

    // Check the indentation by looked at the first few characters of each line's message
    {
      EXPECT_EQ(history[0].at(logTextStartColumn + 0), u8'N'); // ...ot indented

      EXPECT_EQ(history[1].at(logTextStartColumn + 0), u8'1');
      EXPECT_EQ(history[1].at(logTextStartColumn + 1), u8'2');
      EXPECT_EQ(history[1].at(logTextStartColumn + 2), u8'3');
      EXPECT_EQ(history[1].at(logTextStartColumn + 3), u8'4');
      EXPECT_EQ(history[1].at(logTextStartColumn + 4), u8'5');
      EXPECT_EQ(history[1].at(logTextStartColumn + 5), u8'W'); // ...arning

      EXPECT_EQ(history[2].at(logTextStartColumn + 0), u8' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 1), u8' ');
      EXPECT_EQ(history[2].at(logTextStartColumn + 2), u8'5');
      EXPECT_EQ(history[2].at(logTextStartColumn + 3), u8'4');
      EXPECT_EQ(history[2].at(logTextStartColumn + 4), u8'3');
      EXPECT_EQ(history[2].at(logTextStartColumn + 5), u8'2');
      EXPECT_EQ(history[2].at(logTextStartColumn + 6), u8'1');
      EXPECT_EQ(history[2].at(logTextStartColumn + 7), u8'E'); // ...rror
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
