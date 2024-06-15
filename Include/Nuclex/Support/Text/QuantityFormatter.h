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

#ifndef NUCLEX_SUPPORT_TEXT_QUANTITYFORMATTER_H
#define NUCLEX_SUPPORT_TEXT_QUANTITYFORMATTER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <optional> // for std::optional
#include <cstdint> // for std::uint32_t, std::int32_t, std::uint64_t, std::int64_t
#include <chrono> // for std::chrono::seconds

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts quantities into human-readable strings</summary>
  class NUCLEX_SUPPORT_TYPE QuantityFormatter {

    /// <summary>Turns a byte count into a human-readable string</summary>
    /// <param name="byteCount">Byte count for which a string will be generated</param>
    /// <param name="useBinaryMagnitudes">
    ///   Whether to output KiB, GiB and TiB, each being 1024 of the next lower unit rather
    ///   than decimal SI units with KB, GB and TB being 1000 of the next lower unit each.
    /// </param>
    public: NUCLEX_SUPPORT_API static std::string StringFromByteCount(
      std::uint64_t byteCount, bool useBinaryMagnitudes = true
    );

    /// <summary>Turns a duration in seconds into a human-readable string</summary>
    /// <param name="duration">Duration for which a string will be generated</param>
    /// <param name="useSimpleFormat">
    ///   If true, the string will spell out the duration as a single number and unit
    ///   (&quot;5.5 minutes&quot;), otherwise, it will indicate the exact duration as
    ///   the number of days, hours, minutes and seconds (&quot;1d 1:34:12&quot;)
    /// </param>
    /// <remarks>
    ///   This is a simple helper with no localization. While the strings generated
    ///   should be universally understood, they will use English terms for the units.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::string StringFromDuration(
      std::chrono::seconds duration, bool useSimpleFormat = true
    );

    // Duration

    // Elapsed time since UTC timestamp

    // Reamining time

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_QUANTITYFORMATTER_H
