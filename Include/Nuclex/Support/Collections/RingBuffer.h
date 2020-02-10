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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_RINGBUFFER_H
#define NUCLEX_SUPPORT_COLLECTIONS_RINGBUFFER_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <vector> // for std::vector
#include <algorithm> // for std::copy_n()
#include <stdexcept> // for std::out_of_range

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A ring buffer that can grow and read/write in batches</summary>
  /// <remarks>
  ///   This differs from std::queue in two ways: 1) it is optimized for a constant
  ///   ring buffer size (i.e. the capacity can grow, but is assumed to settle quickly)
  ///   and 2) it provides efficient batch operations.
  /// </remarks>
  template<typename TItem>
  class RingBuffer {

    /// <summary>Constant used to indicate an invalid index</summary>
    private: static const std::size_t InvalidIndex = static_cast<std::size_t>(-1);

    /// <summary>Initializes a new ring buffer</summary>
    /// <param name="capacity">Storage space in the ring buffer at the beginning</param>
    public: RingBuffer(std::size_t capacity = 256) :
      itemMemory(new std::uint8_t[sizeof(TItem[2]) * getNextPowerOfTwo(capacity) / 2]),
      capacity(getNextPowerOfTwo(capacity)),
      startIndex(InvalidIndex),
      endIndex(InvalidIndex) {}

    /// <summary>Initializes a ring buffer as a copy of another ring buffer</summary>
    /// <param name="other">Other ring buffer that will be copied</param>
    public: RingBuffer(const RingBuffer &other) :
      itemMemory(new std::uint8_t[sizeof(TItem[2]) * getNextPowerOfTwo(other.capacity) / 2]),
      capacity(getNextPowerOfTwo(other.capacity)),
      startIndex(0),
      endIndex(other.Count()) {

      if(other.startIndex == InvalidIndex) {
        this->startIndex = InvalidIndex;
      } else if(other.startIndex < other.endIndex) {

        // Copy all items from first to last
        TItem *sourceAddress = reinterpret_cast<TItem *>(other.itemMemory) + other.startIndex;
        TItem *targetAddress = reinterpret_cast<TItem *>(this->itemMemory);
        for(std::size_t index = 0; index < this->endIndex; ++index) {
          new(targetAddress) TItem(*sourceAddress);
          ++sourceAddress;
          ++targetAddress;
        }

      } else {
        std::size_t segmentItemCount = other.capacity - other.startIndex;
        TItem *targetAddress = reinterpret_cast<TItem *>(this->itemMemory);

        // Copy all items from the start index up to the end of the other buffer
        TItem *sourceAddress = reinterpret_cast<TItem *>(other.itemMemory) + other.startIndex;
        for(std::size_t index = 0; index < segmentItemCount; ++index) {
          new(targetAddress) TItem(*sourceAddress);
          ++sourceAddress;
          ++targetAddress;
        }

        // Copy all items from the beginning of the other buffer up to the end index
        sourceAddress = reinterpret_cast<TItem *>(other.itemMemory);
        for(std::size_t index = 0; index < other.endIndex; ++index) {
          new(targetAddress) TItem(*sourceAddress);
          ++sourceAddress;
          ++targetAddress;
        }
      }
    }

    /// <summary>Initializes a ring buffer taking over another ring buffer</summary>
    /// <param name="other">Other ring buffer that will be taken over</param>
    public: RingBuffer(RingBuffer &&other) :
      itemMemory(other.itemMemory),
      capacity(other.capacity),
      startIndex(other.startIndex),
      endIndex(other.endIndex) {
      other.itemMemory = nullptr;
#if !defined(NDEBUG)
      other.startIndex = InvalidIndex;
#endif
    }

    /// <summary>Destroys the ring buffer and all items in it</summary>
    public: ~RingBuffer() {
      if(this->itemMemory != nullptr) { // Can be NULL if container donated its guts

        // If the buffer contains items, they, too, need to be destroyed
        if(this->startIndex != InvalidIndex) {

          // If the ring buffer is linear, simply destroy the items from start to end
          if(this->startIndex < this->endIndex) {
            TItem *address = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
            for(std::size_t index = this->startIndex; index < this->endIndex; ++index) {
              address->~TItem();
            }
          } else { // If the ring buffer is wrapped, both segments need to be destroyed
            std::size_t segmentItemCount = this->capacity - this->startIndex;

            // Destroy all items in the older segment
            TItem *address = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
            for(std::size_t index = 0; index < segmentItemCount; ++index) {
              address->~TItem();
            }

            // Destroy all items in the younger segment
            address = reinterpret_cast<TItem *>(this->itemMemory);
            for(std::size_t index = 0; index < this->endIndex; ++index) {
              address->~TItem();
            }
          }
        }

        delete this->itemMemory;

      }
    }

    /// <summary>Looks up the number of items the ring buffer has allocated memory for</summary>
    /// <returns>The number of items the ring buffer has reserved space for</returns>
    /// <remarks>
    ///   Just like std::vector::capacity(), this is not a limit. If the capacity is
    ///   exceeded, the ring buffer will allocate a large memory block and use that one.
    /// </remarks>
    public: std::size_t GetCapacity() const {
      return this->capacity;
    }

    /// <summary>Counts the number of items currently stored in the ring buffer</summary>
    public: std::size_t Count() const {
      if(this->startIndex == InvalidIndex) { // Empty
        return 0;
      } else if(this->startIndex < this->endIndex) { // Items linear
        return this->endIndex - this->startIndex;
      } else { // Items wrapped around
        return this->endIndex + (this->capacity - this->startIndex);
      }
    }

    /// <summary>Appends items to the end of the ring buffer</summary>
    /// <param name="items">Items that will be added to the ring buffer</param>
    /// <param name="count">Number of items that will be added</param>
    public: void Append(const TItem *items, std::size_t count) {
      if(this->startIndex == InvalidIndex) {
        appendToEmpty(items, count);
      } else if(this->endIndex > this->startIndex) {
        appendToLinear(items, count);
      } else {
        appendToWrapped(items, count);
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    public: void Dequeue(TItem *items, std::size_t count) {
      if(this->startIndex == InvalidIndex) {
        if(count > 0) {
          throw std::logic_error(u8"Ring buffer contains fewer items than requested");
        }
      } else if(this->endIndex > this->startIndex) {
        dequeueFromLinear(items, count);
      } else {
        dequeueFromWrapped(items, count);
      }
    }

    /// <summary>Appends items to an empty ring buffer</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="count">Number of items that will be appended</param>
    private: void appendToEmpty(const TItem *items, std::size_t count) {
#if defined(NUCLEX_SUPPORT_CXX20)
      if(count > this->capacity) [[unlikely]] {
#else
      if(count > this->capacity) {
#endif
        delete[] this->itemMemory;
        this->capacity = getNextPowerOfTwo(count);
        this->itemMemory = new std::uint8_t[sizeof(TItem[2]) * capacity / 2];
      }

      std::copy_n(items, count, reinterpret_cast<TItem *>(this->itemMemory));
      this->startIndex = 0;
      this->endIndex = count;
    }

    /// <summary>Appends items to a ring buffer with items stored linearly</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="count">Number of items that will be appended</param>
    private: void appendToLinear(const TItem *items, std::size_t count) {
      std::size_t remainingSegmentItemCount = this->capacity - this->endIndex;
      if(remainingSegmentItemCount >= count) { // New data fits between end and capacity
        std::copy_n(
          items,
          count,
          reinterpret_cast<TItem *>(this->itemMemory) + this->endIndex
        );
        this->endIndex += count;
      } else { // New data must be wrapped or ring buffer needs to be extended
        std::size_t remainingItemCount = remainingSegmentItemCount + this->startIndex;
#if defined(NUCLEX_SUPPORT_CXX20)
        if(remainingItemCount >= count) [[likely]] {
#else
        if(remainingItemCount >= count) {
#endif
          std::copy_n(
            items,
            remainingSegmentItemCount,
            reinterpret_cast<TItem *>(this->itemMemory) + this->endIndex
          );
          this->endIndex = count - remainingSegmentItemCount;
          std::copy_n(
            items + remainingSegmentItemCount,
            this->endIndex,
            reinterpret_cast<TItem *>(this->itemMemory)
          );
        } else { // New data doesn't fit, ring buffer needs to be extended
          std::size_t oldItemCount = this->endIndex - this->startIndex;

          // Allocate new memory for the items
          {
            std::size_t newCapacity = getNextPowerOfTwo(oldItemCount + count);
            std::uint8_t *newItemMemory = new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2];

            // Move-construct the items into the new memory block and destroy them
            // in their old memory block
            {
              TItem *oldAddress = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
              TItem *newAddress = reinterpret_cast<TItem *>(newItemMemory);
              for(std::size_t index = this->startIndex; index < this->endIndex; ++index) {
                new(newAddress) TItem(std::move(*oldAddress));
                oldAddress->~TItem();
                ++oldAddress;
                ++newAddress;
              }
            }

            // Move completed, we can free the old memory block early
            delete[] this->itemMemory;
            this->itemMemory = newItemMemory;
            this->capacity = newCapacity;
          }

          std::copy_n(
            items,
            count,
            reinterpret_cast<TItem *>(this->itemMemory) + oldItemCount
          );

          this->startIndex = 0;
          this->endIndex = oldItemCount + count;
        }
      }
    }

    /// <summary>Appends items to a ring buffer with items that have wrapped around</summary>
    /// <param name="items">Items that will be appended to the ring buffer</param>
    /// <param name="count">Number of items that will be appended</param>
    private: void appendToWrapped(const TItem *items, std::size_t count) {
      std::size_t remainingItemCount = this->startIndex - this->endIndex;
#if defined(NUCLEX_SUPPORT_CXX20)
      if(count > remainingItemCount) [[likely]] { // New data fits, simplest case there is
#else
      if(count > remainingItemCount) { // New data fits, simplest case there is
#endif
        std::copy_n(
          items,
          count,
          reinterpret_cast<TItem *>(this->itemMemory) + this->endIndex
        );
        this->endIndex += count;
      } else { // New data doesn't fit, ring buffer needs to be extended
        std::size_t oldItemCount = this->capacity - remainingItemCount;

        // Allocate new memory for the items
        {
          std::size_t newCapacity = getNextPowerOfTwo(oldItemCount + count);
          std::uint8_t *newItemMemory = new std::uint8_t[sizeof(TItem[2]) * newCapacity / 2];

          // Move-construct the items into the new memory block and destroy them
          // in their old memory block
          {
            TItem *newAddress = reinterpret_cast<TItem *>(newItemMemory);

            // Move the items on the older segment into the new buffer
            TItem *oldAddress = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
            for(std::size_t index = this->startIndex; index < this->capacity; ++index) {
              new(newAddress) TItem(std::move(*oldAddress));
              oldAddress->~TItem();
              ++oldAddress;
              ++newAddress;
            }

            // Move the items on the younger segment into the new buffer
            oldAddress = reinterpret_cast<TItem *>(this->itemMemory);
            for(std::size_t index = 0; index < this->endIndex; ++index) {
              new(newAddress) TItem(std::move(*oldAddress));
              oldAddress->~TItem();
              ++oldAddress;
              ++newAddress;
            }
          }

          // Move completed, we can free the old memory block early
          delete[] this->itemMemory;
          this->itemMemory = newItemMemory;
          this->capacity = newCapacity;
        }

        std::copy_n(
          items,
          count,
          reinterpret_cast<TItem *>(this->itemMemory) + oldItemCount
        );

        this->startIndex = 0;
        this->endIndex = oldItemCount + count;
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    private: void dequeueFromLinear(TItem *items, std::size_t count) {
      std::size_t availableItemCount = this->endIndex - this->startIndex;
      if(availableItemCount >= count) {
        TItem *sourceAddress = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
        for(std::size_t index = 0; index < count; ++index) {
          new(items) TItem(std::move(*sourceAddress));
          sourceAddress->~TItem();
          ++sourceAddress;
          ++items;
        }
        if(count == availableItemCount) {
          this->startIndex = InvalidIndex;
        } else {
          this->startIndex += count;
        }
      } else {
        throw std::logic_error(u8"Ring buffer contains fewer items than requested");
      }
    }

    /// <summary>Removes items from the beginning of the ring buffer</summary>
    /// <param name="items">Buffer in which the dequeued items will be stored</param>
    /// <param name="count">Number of items that will be dequeued</param>
    private: void dequeueFromWrapped(TItem *items, std::size_t count) {
      std::size_t availableSegmentItemCount = this->capacity - this->startIndex;
      if(availableSegmentItemCount >= count) { // Enough data in older segment
        TItem *sourceAddress = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
        for(std::size_t index = 0; index < count; ++index) {
          new(items) TItem(std::move(*sourceAddress));
          sourceAddress->~TItem();
          ++sourceAddress;
          ++items;
        }

        // Was all data in the segment consumed?
        if(count == availableSegmentItemCount) {
          this->startIndex = 0;
        } else {
          this->startIndex += count;
        }
      } else { // The older segment alone does not have enough data, check younger segment
        std::size_t availableItemCount = availableSegmentItemCount + this->endIndex;
        if(availableItemCount >= count) { // Is there enough data with both segments together?

          // Move the items from the older segment into the caller-provided buffer
          TItem *sourceAddress = reinterpret_cast<TItem *>(this->itemMemory) + this->startIndex;
          for(std::size_t index = 0; index < availableSegmentItemCount; ++index) {
            new(items) TItem(std::move(*sourceAddress));
            sourceAddress->~TItem();
            ++sourceAddress;
            ++items;
          }

          count -= availableSegmentItemCount;

          // Move some or all items from the younger segment into the caller-provided buffer
          sourceAddress = reinterpret_cast<TItem *>(this->itemMemory);
          for(std::size_t index = 0; index < count; ++index) {
            new(items) TItem(std::move(*sourceAddress));
            sourceAddress->~TItem();
            ++sourceAddress;
            ++items;
          }

          // If all data was taken from the buffer, then the ring buffer is empty now
          if(count == availableItemCount - availableSegmentItemCount) {
            this->startIndex = InvalidIndex;
          } else { // Otherwise, the new start index is one past the last read position
            this->startIndex = count;
          }
        } else { // There is insufficient data in the ring buffer
          throw std::logic_error(u8"Ring buffer contains fewer items than requested");
        }
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

    /// <summary>Holds the items stored in the ring buffer</summary>
    private: std::uint8_t *itemMemory;
    /// <summary>Number of items the ring buffer can currently hold</summary>
    private: std::size_t capacity;
    /// <summary>Index of the first item in the ring buffer</summary>
    private: std::size_t startIndex;
    /// <summary>Index one past the last item</summary>
    private: std::size_t endIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_RINGBUFFER_H
