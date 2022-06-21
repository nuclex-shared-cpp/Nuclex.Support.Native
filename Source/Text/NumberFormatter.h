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

#ifndef NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H
#define NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H

#include "Nuclex/Support/Config.h"
#include <cstdint>

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

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char *FormatInteger(std::uint32_t value, char *buffer /* [10] */);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char *FormatInteger(std::int32_t value, char *buffer /* [11] */);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char *FormatInteger(std::uint64_t value, char *buffer /* [20] */);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the digits of an integer as UTF-8 characters into a buffer</summary>
  /// <param name="value">Value that will be turned into a string</param>
  /// <param name="buffer">Buffer into which the characters will be written</param>
  /// <returns>A pointer to one character past the last character written</returns>
  /// <remarks>
  ///   This does not append a terminating zero to the buffer.
  /// </remarks>
  char *FormatInteger(std::int64_t value, char *buffer /* [20] */);

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
  char *FormatFloat(float value, char *buffer /* [46] */);

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
  char *FormatFloat(double value, char *buffer /* [325] */);

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_NUMBERFORMATTER_H
