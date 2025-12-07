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

#ifndef NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H
#define NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H

#include "Nuclex/Support/Config.h"
#include <cstdint> // for std::uint32_t, std::int32_t, std::uint64_t, std::int64_t

//
// Data type       |   Number of mantissa bits     |   Smallest possible exponent (radix 10)
//                 |                               |
//   float         |             24                |               -125 / -37
//   float32       |             24                |               -125 / -37
//   float32x      |             53                |              -1021 / -307
//   float64       |             53                |              -1021 / -307
//   float64x      |             64                |             -16381 / (-4931)
//   long double   |             64                |             -16381 / (-4931)
//   float128      |            113                |             -16381 / (-4931)
//
// Longest possible string in exponential notation for a single
//   -1.09045365E-32
//   = 42 characters when written out
//   (but an internet search claims 46)
//
// Longest possible string in exponential notation for a double
//   -2.225073858507202E-308
//   = 325 characters when written out
//

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Table of the numbers 00 .. 99 as a flat array</summary>
  /// <remarks>
  ///   Used for James Edward Anhalt III.'s integer formatting technique where two digits
  ///   are converted at once, among other tricks.
  /// </remarks>
  extern const char8_t Radix100[200];

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <param name="value">Value that will be turned into a string</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatInteger(char8_t *buffer /* [10] */, std::uint32_t value);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <param name="value">Value that will be turned into a string</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatInteger(char8_t *buffer /* [11] */, std::int32_t value);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <param name="value">Value that will be turned into a string</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatInteger(char8_t *buffer /* [20] */, std::uint64_t value);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <param name="value">Value that will be turned into a string</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatInteger(char8_t *buffer /* [20] */, std::int64_t value);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Writes the digits of a floating point value as UTF-8 characters into a buffer
  /// </summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   Always uses non-exponential notation.
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatFloat(char8_t *buffer /* [46] */, float value);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Writes the digits of a floating point value as UTF-8 characters into a buffer
  /// </summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   Always uses non-exponential notation.
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char8_t *FormatFloat(char8_t *buffer /* [325] */, double value);

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H
