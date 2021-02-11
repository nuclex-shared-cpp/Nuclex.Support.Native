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

#if !defined(NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H)
#error This file must be included through via ConcurrentRingBuffer.h
#endif

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size list that can safely be used from multiple threads</summary>
  ///   <para>
  ///     <strong>Thread safety:</strong> any thread depending on selected specialization
  ///   </para>
  ///   <para>
  ///     <strong>Container type:</strong> bounded ring buffer
  ///   </para>
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::MultipleProducersSingleConsumer> {

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity + 1),
      itemMemory(nullptr),
      itemStatus(nullptr),
      freeSlotCount(capacity),
      readIndex(0),
      writeIndex(0),
      occupiedIndex(0) {
      std::uint8_t *buffer = new std::uint8_t[sizeof(TElement[2]) * (capacity + 1U) / 2U];
      {
        auto itemMemoryDeleter = ON_SCOPE_EXIT_TRANSACTION {
          delete[] buffer;
        };
        this->itemStatus = new std::atomic<std::uint8_t>[capacity + 1U];
        itemMemoryDeleter.Commit();
      }
      this->itemMemory = reinterpret_cast<TElement *>(buffer);

      // Initialize the status of all items
      for(std::size_t index = 0; index < capacity + 1; ++index) {
        this->itemStatus[index].store(0, std::memory_order_relaxed);
      }
    }
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentRingBuffer() {
      if constexpr(!std::is_trivially_destructible<TElement>::value) {
/*      
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        int safeWriteIndex = this->writeIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        while(safeReadIndex != positiveModulo(safeWriteIndex)) {
          reinterpret_cast<TElement *>(this->itemMemory)[safeReadIndex].~TElement();
          safeReadIndex = (safeReadIndex + 1) % this->capacity;
        }
*/
      }

      delete[] this->itemStatus;
#if !defined(NDEBUG)
      this->itemStatus = nullptr;
#endif
      delete[] reinterpret_cast<std::uint8_t *>(this->itemMemory);
#if !defined(NDEBUG)
      this->itemMemory = nullptr;
#endif
    }

    /// <summary>Tries to append the specified element to the queue</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(const TElement &element) {

      // Try to reserve a slot. If the queue is full, the value will be zero (or even less,
      // if highly contested), in which case we just hand the unusable slot back and return.
      {
        int safeFreeSlotCount = this->freeSlotCount.fetch_sub(
          1, std::memory_order_consume // consume: if() below carries dependency
        );
        if(safeFreeSlotCount < 1) { // 1 because fetch_sub() returns the previous value
          this->freeSlotCount.fetch_add(1, std::memory_order_release);
          return false;
        }
      }

      // If we reach this spot, we know there was at least 1 slot free in the queue and we
      // just captured it (i.e. no other thread will cause less than 1 slot to remain free).
      // So we just need to 'take' a slot index from the occupied index list
      std::size_t targetSlotIndex;
      {
        int safeOccupiedIndex = this->occupiedIndex.fetch_add(
          1, std::memory_order_consume // consume: if() below carries dependency
        );
        if(safeOccupiedIndex > 0) {
          if(static_cast<std::size_t>(safeOccupiedIndex) >= this->capacity) {
            this->occupiedIndex.fetch_sub(this->capacity, std::memory_order_relaxed);
          }
        }
        targetSlotIndex = positiveModulo(safeOccupiedIndex, this->capacity);
      }

      // Mark the slot as being filled currently for the reading thread
      this->itemStatus[targetSlotIndex].store(1, std::memory_order_release);

      // AARGH!
      //
      // - If the consumer thread uses occupiedSlotIndex, it'll see occupied slots
      //   before the status is updated (race between constructing into slot and status update)
      //
      // - If we use writeIndex, it'll be a race on that index, two producing threads
      //   could finish in opposite order they started, so second thread increments
      //   write index but thereby exposes item still being filled
      //
      // ? If the read thread sets the item status to free (which it must), is this problem
      //   avoided in both cases?
      //

      // Copy the item into the slot. If its copy constructor throws, the slot must be
      // marked as broken so the reading thread will skip it.
      {
        auto brokenSlotScope = ON_SCOPE_EXIT_TRANSACTION {
          this->itemStatus[targetSlotIndex].store(3, std::memory_order_release);
        };
        new(this->itemMemory + targetSlotIndex) TElement(element);
        brokenSlotScope.Commit();
      }



      // Item was appended!
      return true;

    }

    /// <summary>Returns the maximum number of items the queue can hold</summary>
    /// <returns>The maximum number of items the queue can hold</returns>
    public: std::size_t GetCapacity() const { return this->capacity - 1U; }

    /// <summary>Performs the modulo operation, but returns 0..divisor-1</summary>
    /// <param name="value">Value for which the positive modulo will be calculated</param>
    /// <param name="divisor">Divisor of which the remainder will be calculated</param>
    /// <returns>The positive division remainder of the specified value</returns>
    /// <remarks>
    ///   There are various tricks to achieve this without branching, but they're all slower.
    ///   Reason: x86, amd64 and ARM CPUs have conditional move instructions, allowing cases
    ///   like this one to execute without branching at the machine code level.
    /// </remarks>
    private: static std::size_t positiveModulo(int value, int divisor) {
      value %= divisor;
      if(value < 0) {
        return static_cast<std::size_t>(value + divisor);
      } else {
        return static_cast<std::size_t>(value);
      }
    }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: const std::size_t capacity;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    private: TElement *itemMemory;
    /// <summary>Status of items in buffer, 0: empty, 1: filling, 2: present, 3: gap</summary>
    private: std::atomic<std::uint8_t> *itemStatus;

    /// <summary>Number of free slots the queue can store elements in</summary>
    /// <remarks>
    ///   <para>
    ///     This allows the <see cref="TryAppend" /> method to know whether a slot will be free
    ///     after the current write index, eliminating the whole C-A-S loop. While reserving,
    ///     the value will be blindly decremented, then checked and - if negative - incremented
    ///     back up.
    ///   </para>
    ///   <para>
    ///     Also important is that this counts slots, not items. If a constructor throws during
    ///     an append operation, the slot will remain occupied (because it can't be safely
    ///     returned), but no item will be in it.
    ///   </para>
    /// </remarks>
    private: std::atomic<int> freeSlotCount;

    /// <summary>Index from which the next item will be read</summary>
    private: std::atomic<std::size_t> readIndex;

    /// <summary>Index at which the most recently written item is stored</summary>
    /// <remarks>
    ///   Notice that contrary to normal practice, this does not point one past the last
    ///   item (i.e. to the position of the next write), but is the index of the last item
    ///   that has been stored in the buffer. The lock-free synchronization is easier this way.
    /// </remarks>
    private: std::atomic<int> writeIndex;
    /// <summary>Like write index, but for the next item to be filled</summary>
    private: std::atomic<int> occupiedIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
