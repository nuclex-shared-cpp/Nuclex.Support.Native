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

#include "Nuclex/Support/Text/QuantityFormatter.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintBinaryByteCount) {
    std::string smallBytes = QuantityFormatter::stringFromByteCount(234);
    EXPECT_NE(smallBytes.find(u8"234"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::string::npos);

    std::string bigBytes = QuantityFormatter::stringFromByteCount(789);
    EXPECT_NE(bigBytes.find(u8"0.8"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"KiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(324'123);
    EXPECT_NE(smallBytes.find(u8"316.5"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"KiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(876'456);
    EXPECT_NE(bigBytes.find(u8"0.8"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"MiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(139'432'174);
    EXPECT_NE(smallBytes.find(u8"133.0"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"MiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(977'341'931);
    EXPECT_NE(bigBytes.find(u8"0.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"GiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(412'523'934'812);
    EXPECT_NE(smallBytes.find(u8"384.2"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"GiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(634'839'012'517);
    EXPECT_NE(bigBytes.find(u8"0.6"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"TiB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(347'104'194'387'594);
    EXPECT_NE(smallBytes.find(u8"315.7"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"TiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(893'270'909'743'209);
    EXPECT_NE(bigBytes.find(u8"0.8"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PiB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(936'582'932'385'623'894);
    EXPECT_NE(bigBytes.find(u8"831.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PiB"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(QuantityFormatterTest, CanPrintMetricByteCount) {
    std::string smallBytes = QuantityFormatter::stringFromByteCount(234, false);
    EXPECT_NE(smallBytes.find(u8"234"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"bytes"), std::string::npos);

    std::string bigBytes = QuantityFormatter::stringFromByteCount(789, false);
    EXPECT_NE(bigBytes.find(u8"0.8"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"KB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(324'123, false);
    EXPECT_NE(smallBytes.find(u8"324.1"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"KB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(876'456, false);
    EXPECT_NE(bigBytes.find(u8"0.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"MB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(139'432'174, false);
    EXPECT_NE(smallBytes.find(u8"139.4"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"MB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(977'341'931, false);
    EXPECT_NE(bigBytes.find(u8"1.0"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"GB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(412'523'934'812, false);
    EXPECT_NE(smallBytes.find(u8"412.5"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"GB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(634'839'012'517, false);
    EXPECT_NE(bigBytes.find(u8"0.6"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"TB"), std::string::npos);

    smallBytes = QuantityFormatter::stringFromByteCount(347'104'194'387'594, false);
    EXPECT_NE(smallBytes.find(u8"347.1"), std::string::npos);
    EXPECT_NE(smallBytes.find(u8"TB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(893'270'909'743'209, false);
    EXPECT_NE(bigBytes.find(u8"0.9"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PB"), std::string::npos);

    bigBytes = QuantityFormatter::stringFromByteCount(936'582'932'385'623'894, false);
    EXPECT_NE(bigBytes.find(u8"936.6"), std::string::npos);
    EXPECT_NE(bigBytes.find(u8"PB"), std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
