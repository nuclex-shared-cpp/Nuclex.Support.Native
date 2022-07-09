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
#include "Nuclex/Support/Text/LexicalAppend.h"

#include <cstdlib> // for std::strtoul(), std::strtoull(), std::strtol(), std::strtoll()

namespace {

  // ------------------------------------------------------------------------------------------- //

  void appendWithOneDecimalDigit(std::string &target, std::size_t numberTimesTen) {
    Nuclex::Support::Text::lexical_append(target, numberTimesTen / 10);
    target.push_back(u8'.');
    Nuclex::Support::Text::lexical_append(target, numberTimesTen % 10);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::string QuantityFormatter::stringFromByteCount(
    std::uint64_t byteCount, bool useBinaryMagnitudes /* = true */
  ) {
    std::string result;

    // CHECK: Maybe taking 64 as the rounding point for binary bits is silly?

    if(useBinaryMagnitudes) {
      if(byteCount < 1'023'936) { // up to 1024 KiB
        if(byteCount < 1'000) { // less than 1000 bytes
          result.reserve(9);
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 1000 bytes to 1024 KiB
          result.reserve(10);
          appendWithOneDecimalDigit(result, (byteCount + 64) * 10 / 1'024);
          const char *bytesText = u8" KiB";
          result.append(bytesText, bytesText + 4);
        }
      } else if(byteCount < 1'073'674'715'136) { // up to 1024 GiB
        if(byteCount < 1'048'510'464) { // less than 1024 MiB
          appendWithOneDecimalDigit(result, (byteCount + 65'536) * 10 / 1'048'576);
          const char *bytesText = u8" MiB";
          result.append(bytesText, bytesText + 4);
        } else { // 1024 MiB to 1024 GiB
          appendWithOneDecimalDigit(result, (byteCount + 67'108'864) * 10 / 1'073'741'824);
          const char *bytesText = u8" GiB";
          result.append(bytesText, bytesText + 4);
        }
      } else {
        if(byteCount < 1'099'442'908'299'264) { // less than 1024 TiB
          appendWithOneDecimalDigit(result, (byteCount + 68'719'476'736) * 10 / 1'099'511'627'776);
          const char *bytesText = u8" TiB";
          result.append(bytesText, bytesText + 4);
        } else { // more than 1024 TiB
          appendWithOneDecimalDigit(result, (byteCount + 70'368'744'177'664) * 10 / 1'125'899'906'842'624);
          const char *bytesText = u8" PiB";
          result.append(bytesText, bytesText + 4);
        }
      }
    } else { // metric magnitudes rather than binary
      result.reserve(9);

      if(byteCount < 999'950) { // up to 1000 KB
        if(byteCount < 1000) { // less than 1000 bytes
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 512 bytes to 500 KB
          appendWithOneDecimalDigit(
            result, (byteCount + 50) * 10 / 1'000
          );
          const char *bytesText = u8" KB";
          result.append(bytesText, bytesText + 3);
        }
      } else if(byteCount < 999'950'000'000) { // up to 1000 GB
        if(byteCount < 999'950'000) { // less than 1000 MB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000) * 10 / 1'000'000
          );
          const char *bytesText = u8" MB";
          result.append(bytesText, bytesText + 3);
        } else { // 1000 MB to 1000 GB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000) * 10 / 1'000'000'000
          );
          const char *bytesText = u8" GB";
          result.append(bytesText, bytesText + 3);
        }
      } else {
        if(byteCount < 999'950'000'000'000) { // less than 1000 TB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000) * 10 / 1'000'000'000'000
          );
          const char *bytesText = u8" TB";
          result.append(bytesText, bytesText + 3);
        } else { // more than 1000 TB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000'000) * 10 / 1'000'000'000'000'000
          );
          const char *bytesText = u8" PB";
          result.append(bytesText, bytesText + 3);
        }
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string QuantityFormatter::stringFromDuration(
    std::chrono::seconds duration, bool useLongFormat /* = true */
  ) {
    std::string result;

    if(useLongFormat) {
      if(duration < std::chrono::seconds(3'597)) { // less than an hour?
        if(duration < std::chrono::seconds(60)) { // less than a minute?
          result.reserve(10);
          lexical_append(result, duration.count());
          const char *bytesText = u8" seconds";
          result.append(bytesText, bytesText + 8);
        } else { // a minute to an hour
          result.reserve(12);
          appendWithOneDecimalDigit(result, (duration.count() + 3) * 10 / 60);
          const char *bytesText = u8" minutes";
          result.append(bytesText, bytesText + 8);
        }
      } else if(duration < std::chrono::seconds(1'205'280)) { // less than two weeks?
        if(duration < std::chrono::seconds(86'220)) { // less than a day?
          result.reserve(10);
          appendWithOneDecimalDigit(result, (duration.count() + 180) * 10 / 3'600);
          const char *bytesText = u8" hours";
          result.append(bytesText, bytesText + 6);
        } else { // a day to two weeks
          result.reserve(9);
          appendWithOneDecimalDigit(result, (duration.count() + 4'320) * 10 / 86'400);
          const char *bytesText = u8" days";
          result.append(bytesText, bytesText + 5);
        }
      } else if(duration < std::chrono::seconds(31'490'640)) { // less than a year?
        if(duration < std::chrono::seconds(4'808'160)) { // less than eight weeks?
          result.reserve(10);
          appendWithOneDecimalDigit(result, (duration.count() + 30'240) * 10 / 604'800);
          const char *bytesText = u8" weeks";
          result.append(bytesText, bytesText + 6);
        } else { // eight weeks to a year
          result.reserve(11);
          appendWithOneDecimalDigit(result, (duration.count() + 131'760) * 10 / 2'635'200);
          const char *bytesText = u8" months";
          result.append(bytesText, bytesText + 7);
        }
      } else { // more than a year
        result.reserve(12);
        appendWithOneDecimalDigit(result, (duration.count() + 1'576'800) * 10 / 31'536'000);
        const char *bytesText = u8" years";
        result.append(bytesText, bytesText + 6);
      }
    } else {
      result.append(u8"soon");
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
