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

#include "Nuclex/Support/Text/QuantityFormatter.h"

#include <gtest/gtest.h>

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintMetricByteCount) {
    std::u8string smallBytes = QuantityFormatter::StringFromByteCount(234, false);
    EXPECT_NE(smallBytes.find(u8"234"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::u8string::npos);

    std::u8string bigBytes = QuantityFormatter::StringFromByteCount(789, false);
    EXPECT_NE(bigBytes.find(u8"789"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"bytes"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(324'123, false);
    EXPECT_NE(smallBytes.find(u8"324.1"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"KB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(876'456, false);
    EXPECT_NE(bigBytes.find(u8"876.5"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"KB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(139'432'174, false);
    EXPECT_NE(smallBytes.find(u8"139.4"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"MB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(977'341'931, false);
    EXPECT_NE(bigBytes.find(u8"977.3"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"MB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(412'523'934'812, false);
    EXPECT_NE(smallBytes.find(u8"412.5"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"GB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(634'839'012'517, false);
    EXPECT_NE(bigBytes.find(u8"634.8"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"GB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(347'104'194'387'594, false);
    EXPECT_NE(smallBytes.find(u8"347.1"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"TB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(893'270'909'743'209, false);
    EXPECT_NE(bigBytes.find(u8"893.3"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"TB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(36'093'248'903'249'082, false);
    EXPECT_NE(smallBytes.find(u8"36.1"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"PB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(936'582'932'385'623'894, false);
    EXPECT_NE(bigBytes.find(u8"936.6"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"PB"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintBinaryByteCount) {
    std::u8string smallBytes = QuantityFormatter::StringFromByteCount(234);
    EXPECT_NE(smallBytes.find(u8"234"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::u8string::npos);

    std::u8string bigBytes = QuantityFormatter::StringFromByteCount(789);
    EXPECT_NE(bigBytes.find(u8"789"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"bytes"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(324'123);
    EXPECT_NE(smallBytes.find(u8"316.5"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"KiB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(876'456);
    EXPECT_NE(bigBytes.find(u8"855.9"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"KiB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(139'432'174);
    EXPECT_NE(smallBytes.find(u8"133.0"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"MiB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(977'341'931);
    EXPECT_NE(bigBytes.find(u8"932.1"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"MiB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(412'523'934'812);
    EXPECT_NE(smallBytes.find(u8"384.2"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"GiB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(634'839'012'517);
    EXPECT_NE(bigBytes.find(u8"591.3"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"GiB"), std::u8string::npos);

    smallBytes = QuantityFormatter::StringFromByteCount(347'104'194'387'594);
    EXPECT_NE(smallBytes.find(u8"315.7"), std::u8string::npos);
    EXPECT_NE(smallBytes.find(u8"TiB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(893'270'909'743'209);
    EXPECT_NE(bigBytes.find(u8"812.4"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"TiB"), std::u8string::npos);

    bigBytes = QuantityFormatter::StringFromByteCount(936'582'932'385'623'894);
    EXPECT_NE(bigBytes.find(u8"831.9"), std::u8string::npos);
    EXPECT_NE(bigBytes.find(u8"PiB"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintSimpleDuration) {
    std::u8string smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(18));
    EXPECT_NE(smallDuration.find(u8"18"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"seconds"), std::u8string::npos);

    std::u8string bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(46));
    EXPECT_NE(bigDuration.find(u8"46"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"seconds"), std::u8string::npos);

    smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(1'423));
    EXPECT_NE(smallDuration.find(u8"23.7"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"minutes"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(3'390));
    EXPECT_NE(bigDuration.find(u8"56.5"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"minutes"), std::u8string::npos);

    smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(28'123));
    EXPECT_NE(smallDuration.find(u8"7.8"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"hours"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(67'803));
    EXPECT_NE(bigDuration.find(u8"18.8"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"hours"), std::u8string::npos);

    smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(248'824));
    EXPECT_NE(smallDuration.find(u8"2.9"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"days"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(432'430));
    EXPECT_NE(bigDuration.find(u8"5.0"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"days"), std::u8string::npos);

    smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(2'113'029));
    EXPECT_NE(smallDuration.find(u8"3.5"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"weeks"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(4'431'029));
    EXPECT_NE(bigDuration.find(u8"7.3"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"weeks"), std::u8string::npos);

    smallDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(13'329'083));
    EXPECT_NE(smallDuration.find(u8"5.1"), std::u8string::npos);
    EXPECT_NE(smallDuration.find(u8"months"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(30'382'901));
    EXPECT_NE(bigDuration.find(u8"11.5"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"months"), std::u8string::npos);

    bigDuration = QuantityFormatter::StringFromDuration(std::chrono::seconds(130'382'901));
    EXPECT_NE(bigDuration.find(u8"4.1"), std::u8string::npos);
    EXPECT_NE(bigDuration.find(u8"years"), std::u8string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintFullDuration) {
    std::u8string smallDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(18), false
    );
    EXPECT_EQ(smallDuration, std::u8string(u8"18s"));

    std::u8string bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(46), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"46s"));

    smallDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(1'423), false
    );
    EXPECT_EQ(smallDuration, std::u8string(u8"23m43s"));

    bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(3'390), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"56m30s"));

    smallDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(28'123), false
    );
    EXPECT_EQ(smallDuration, std::u8string(u8"7h48m43s"));

    bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(67'803), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"18h50m03s"));

    smallDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(248'824), false
    );
    EXPECT_EQ(smallDuration, std::u8string(u8"2d 21h07m04s"));

    bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(432'430), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"5d 0h07m10s"));

    smallDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(13'329'083), false
    );
    EXPECT_EQ(smallDuration, std::u8string(u8"5m1d 18h31m23s"));

    bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(30'382'901), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"11m16d 3h41m41s"));

    bigDuration = QuantityFormatter::StringFromDuration(
      std::chrono::seconds(130'382'901), false
    );
    EXPECT_EQ(bigDuration, std::u8string(u8"4y1m20d 15h52m21s"));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text
