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
    result.reserve(9);

    // CHECK: Maybe taking 64 as the rounding point for binary bits is silly?

    if(useBinaryMagnitudes) {
      if(byteCount < 524'288) { // up to 512 KiB
        if(byteCount < 512) { // less than 512 bytes
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 512 bytes to 512 KiB
          appendWithOneDecimalDigit(result, (byteCount + 64) * 10 / 1'024);
          const char *bytesText = u8" KiB";
          result.append(bytesText, bytesText + 4);
        }
      } else if(byteCount < 549'755'813'888) { // up to 512 GiB
        if(byteCount < 536'870'912) { // less then 512 MiB
          appendWithOneDecimalDigit(result, (byteCount + 65'536) * 10 / 1'048'576);
          const char *bytesText = u8" MiB";
          result.append(bytesText, bytesText + 4);
        } else { // 512 MiB to 512 GiB
          appendWithOneDecimalDigit(result, (byteCount + 67'108'864) * 10 / 1'073'741'824);
          const char *bytesText = u8" GiB";
          result.append(bytesText, bytesText + 4);
        }
      } else { // if(byteCount < 576'460'752'303'423'488) { // up to 512 PiB
        if(byteCount < 562'949'953'421'312) { // less then 512 TiB
          appendWithOneDecimalDigit(result, (byteCount + 68'719'476'736) * 10 / 1'099'511'627'776);
          const char *bytesText = u8" TiB";
          result.append(bytesText, bytesText + 4);
        } else { // 512 TiB to 512 PiB (or infinity)
          appendWithOneDecimalDigit(result, (byteCount + 70'368'744'177'664) * 10 / 1'125'899'906'842'624);
          const char *bytesText = u8" PiB";
          result.append(bytesText, bytesText + 4);
        }
      }
    } else { // metric magnitudes rather than binary
      if(byteCount < 500'000) { // up to 512 KB
        if(byteCount < 500) { // less than 500 bytes
          lexical_append(result, byteCount);
          const char *bytesText = u8" bytes";
          result.append(bytesText, bytesText + 6);
        } else { // 512 bytes to 500 KB
          appendWithOneDecimalDigit(
            result, (byteCount + 50) * 10 / 1'000
          );
          const char *bytesText = u8" KB";
          result.append(bytesText, bytesText + 4);
        }
      } else if(byteCount < 500'000'000'000) { // up to 500 GB
        if(byteCount < 500'000'000) { // less then 500 MB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000) * 10 / 1'000'000
          );
          const char *bytesText = u8" MB";
          result.append(bytesText, bytesText + 4);
        } else { // 500 MB to 500 GB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000) * 10 / 1'000'000'000
          );
          const char *bytesText = u8" GB";
          result.append(bytesText, bytesText + 4);
        }
      } else { // if(byteCount < 500'000'000'000'000'000) { // up to 500 PiB
        if(byteCount < 500'000'000'000'000) { // less then 500 TiB
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000) * 10 / 1'000'000'000'000
          );
          const char *bytesText = u8" TB";
          result.append(bytesText, bytesText + 4);
        } else { // 500 TB to 500 PB (or infinity)
          appendWithOneDecimalDigit(
            result, (byteCount + 50'000'000'000'000) * 10 / 1'000'000'000'000'000
          );
          const char *bytesText = u8" PB";
          result.append(bytesText, bytesText + 4);
        }
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string QuantityFormatter::stringFromDuration(
    std::chrono::seconds duration, bool useLongFormat /* = true */
  ) {
    return u8"Not yet.";
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
