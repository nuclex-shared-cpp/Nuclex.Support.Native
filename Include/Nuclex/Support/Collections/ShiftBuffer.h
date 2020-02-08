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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_SHIFTBUFFER_H
#define NUCLEX_SUPPORT_COLLECTIONS_SHIFTBUFFER_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <vector> // for std::vector
#include <algorithm> // for std::copy_n()
#include <stdexcept> // for std::out_of_range

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A buffer that acts like a ring buffer but guarantees linear memory</summary>
  /// <remarks>
  ///   <para>
  ///     This is a buffer with FIFO batch operations like the ring buffer, but instead of
  ///     wrapping data around, it will keep all data linear. This can be less efficient than
  ///     a ring buffer if there are lots of small reads, but can also be more efficient in
  ///     cases where most (but not all!) of the buffer is removed regularly.
  ///   </para>
  ///   <para>
  ///     It works by simply accumulating data in a linear buffer. Reads simple advance
  ///     the read pointer without freeing space in the buffer. Whenever the wasted space
  ///     in the buffer becomes larger than the space holding waiting data, the waiting
  ///     data is shifted to the front (which can now be done with a non-interecting memory
  ///     move operation)
  ///   </para>
  ///   <para>
  ///     In contrast to a ring buffer, this buffer also allows you to obtain a pointer to
  ///     the data it holds, allowing for extra efficiency if the data can be processed
  ///     directly from a buffer.
  ///   </para>
  /// </remarks>
  template<typename TItem>
  class ShiftBuffer {

    /// <summary>Constant used to indicate an invalid index</summary>
    private: static const std::size_t InvalidIndex = static_cast<std::size_t>(-1);

    /// <summary>Initializes a new shift buffer</summary>
    /// <param name="capacity">Storage space in the shift  buffer at the beginning</param>
    public: ShiftBuffer(std::size_t capacity = 256) :
      itemMemory(new std::uint8_t[sizeof(TItem[2]) * getNextPowerOfTwo(capacity) / 2]),
      capacity(getNextPowerOfTwo(capacity)),
      startIndex(InvalidIndex),
      endIndex(InvalidIndex) {}

    /// <summary>Destroys the shift buffer and all items in it</summary>
    public: ~ShiftBuffer() {
      if(this->itemMemory != nullptr) { // Can be NULL if container donated its guts
        if(this->startIndex != InvalidIndex) {
          TItem *address = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
          for(std::size_t index = this->startIndex; index < this->endIndex; ++index) {
            address->~TItem();
          }
        }

        delete this->itemMemory;
      }
    }

    /// <summary>Looks up the number of items the ring shift has allocated memory for</summary>
    /// <returns>The number of items the ring buffer has reserved space for</returns>
    /// <remarks>
    ///   Just like std::vector::capacity(), this is not a limit. If the capacity is
    ///   exceeded, the shift buffer will allocate a large memory block and use that one.
    /// </remarks>
    public: std::size_t GetCapacity() const {
      return this->capacity;
    }

    /// <summary>Counts the number of items currently stored in the shift buffer</summary>
    public: std::size_t Count() const {
      if(this->startIndex == InvalidIndex) { // Empty
        return 0;
      } else {
        return this->endIndex - this->startIndex;
      }
    }

    /// <summary>Calculates the next power of two for the specified value</summary>
    /// <param name="value">Value of which the next power of two will be calculated</param>
    /// <returns>The next power of two to the specified value</returns>
    private: static std::uint32_t getNextPowerOfTwo(std::uint32_t value) {
      --value;

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;

      return (value + 1);
    }

    /// <summary>Calculates the next power of two for the specified value</summary>
    /// <param name="value">Value of which the next power of two will be calculated</param>
    /// <returns>The next power of two to the specified value</returns>
    private: static std::uint64_t getNextPowerOfTwo(std::uint64_t value) {
      --value;

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;
      value |= value >> 32;

      return (value + 1);
    }

    /// <summary>Holds the items stored in the shift buffer</summary>
    private: std::uint8_t *itemMemory;
    /// <summary>Number of items the shift buffer can currently hold</summary>
    private: std::size_t capacity;
    /// <summary>Index of the first item in the shift buffer</summary>
    private: std::size_t startIndex;
    /// <summary>Index one past the last item</summary>
    private: std::size_t endIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_SHIFTBUFFER_H
