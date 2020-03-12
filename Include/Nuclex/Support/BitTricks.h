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

#ifndef NUCLEX_SUPPORT_BITTRICKS_H
#define NUCLEX_SUPPORT_BITTRICKS_H

#include "Nuclex/Support/Config.h"

#include <cstddef>
#include <cstdint>

// Microsoft compilers need a special header to know their intrinsics
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <intrin.h>
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A few helper methods for bit manipulation</summary>
  class BitTricks {
    /// <summary>Counts the number of leading zero bits in a value</summary>
    /// <param name="value">Value in which the leading zero bits will be couned</param>
    /// <returns>The number of leading zero bits in the value</returns>
    public: static inline constexpr unsigned char CountLeadingZeroBits(std::uint32_t value) {
#if defined(_MSC_VER)
      return ::__lzcnt(value);
#elif defined(__clang__)
      return static_cast<unsigned char>(::__builtin_clz(value));
#elif (defined(__GNUC__) || defined(__GNUG__))
      return static_cast<unsigned char>(::__builtin_clz(value));
#else
      // https://www.chessprogramming.org/BitScan#Bitscan_reverse
      // https://stackoverflow.com/questions/2589096/
      const unsigned char deBruijnBitPosition[32] = {
        31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1,
        23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0
      };

      value |= value >> 1; // first round down to one less than a power of 2
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;

      return deBruijnBitPosition[(value * 0x07C4ACDDU) >> 27];
#endif
    }

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_BITTRICKS_H
