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

#ifndef NUCLEX_SUPPORT_ENDIAN_H
#define NUCLEX_SUPPORT_ENDIAN_H

#include "Nuclex/Support/Config.h"

#include <cstdint> // for std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t

// Microsoft compilers need a special header to know their intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods for dealing with different endianness</summary>
  class NUCLEX_SUPPORT_TYPE Endian {

    /// <summary>Does nothing</summary>
    /// <param name="integer">Integer that will be returned unmodified</param>
    /// <returns>The input value, unmodified</returns>
    public: static std::uint8_t [[nodiscard]] Flip(std::uint8_t integer);

    /// <summary>Reverses the bytes of a 16 bit integer</summary>
    /// <param name="integer">Integer whose bytes will be reversed</param>
    /// <returns>The endian-flipped 16 bit integer</returns>
    public: static std::uint16_t [[nodiscard]] Flip(std::uint16_t integer);

    /// <summary>Reverses the bytes of a 32 bit integer</summary>
    /// <param name="integer">Integer whose bytes will be reversed</param>
    /// <returns>The endian-flipped 32 bit integer</returns>
    public: static std::uint32_t [[nodiscard]] Flip(std::uint32_t integer);

    /// <summary>Reverses the bytes of a 64 bit integer</summary>
    /// <param name="integer">Integer whose bytes will be reversed</param>
    /// <returns>The endian-flipped 64 bit integer</returns>
    public: static std::uint64_t [[nodiscard]] Flip(std::uint64_t integer);

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Class compatible to <see cref="Endian" /> that does nothing</summary>
  class NUCLEX_SUPPORT_TYPE EndianNoop {

    /// <summary>Does nothing</summary>
    /// <param name="integer">Integer that will be returned unmodified</param>
    /// <returns>The input value, unmodified</returns>
    public: static std::uint8_t [[nodiscard]] Flip(std::uint8_t integer);

    /// <summary>Does nothing</summary>
    /// <param name="integer">Integer that will be returned unmodified</param>
    /// <returns>The input value, unmodified</returns>
    public: static std::uint16_t [[nodiscard]] Flip(std::uint16_t integer);

    /// <summary>Does nothing</summary>
    /// <param name="integer">Integer that will be returned unmodified</param>
    /// <returns>The input value, unmodified</returns>
    public: static std::uint32_t [[nodiscard]] Flip(std::uint32_t integer);

    /// <summary>Does nothing</summary>
    /// <param name="integer">Integer that will be returned unmodified</param>
    /// <returns>The input value, unmodified</returns>
    public: static std::uint64_t [[nodiscard]] Flip(std::uint64_t integer);

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LITTLE_ENDIAN)
  /// <summary>Converts from host to network (big endian) byte order</summary>
  typedef Endian NetworkFromHostEndian;
  /// <summary>Converts from network (big endian) to host byte order</summary>
  typedef Endian HostFromNetworkEndian;
#else
  /// <summary>Converts from host to network (big endian) byte order</summary>
  typedef EndianNoop NetworkFromHostEndian;
  /// <summary>Converts from network (big endian) to host byte order</summary>
  typedef EndianNoop HostFromNetworkEndian;
#endif
  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint8_t Endian::Flip(std::uint8_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint16_t Endian::Flip(std::uint16_t integer) {
#if defined(_MSC_VER)
    return _byteswap_ushort(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap16(integer);
#else
    return (integer << 8) | (integer >> 8);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint32_t Endian::Flip(std::uint32_t integer) {
#if defined(_MSC_VER)
    return _byteswap_ulong(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap32(integer);
#else
    return (
      (integer << 24) |
      ((integer & 0x0000FF00) << 8) |
      ((integer & 0x00FF0000) >> 8) |
      (integer >> 24)
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint64_t Endian::Flip(std::uint64_t integer) {
#if defined(_MSC_VER)
    return _byteswap_uint64(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap64(integer);
#else
    return (
      (integer << 56) |
      ((integer & 0x000000000000FF00ULL) << 40) |
      ((integer & 0x0000000000FF0000ULL) << 24) |
      ((integer & 0x00000000FF000000ULL) << 8) |
      ((integer & 0x000000FF00000000ULL) >> 8) |
      ((integer & 0x0000FF0000000000ULL) >> 24) |
      ((integer & 0x00FF000000000000ULL) >> 40) |
      (integer >> 56)
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint8_t EndianNoop::Flip(std::uint8_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint16_t EndianNoop::Flip(std::uint16_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint32_t EndianNoop::Flip(std::uint32_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

  NUCLEX_SUPPORT_ALWAYS_INLINE std::uint64_t EndianNoop::Flip(std::uint64_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_ENDIAN_H
