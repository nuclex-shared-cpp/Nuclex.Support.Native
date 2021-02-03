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
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::SingleProducerSingleConsumer> {

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity + 1),
      readIndex(0),
      writeIndex(0),
      itemMemory(new std::uint8_t(sizeof(TElement[2]) * (capacity + 1) / 2U)) {}
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentRingBuffer() {
      if constexpr(!std::is_trivially_destructible<TElement>::value) {
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        std::size_t safeWriteIndex = this->writeIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        while(safeReadIndex != safeWriteIndex) {
          reinterpret_cast<TElement *>(this->itemMemory)[safeReadIndex].~TElement();
          safeReadIndex = (safeReadIndex + 1) % this->capacity;
        }
        // No updates to read and write index since this is the destructor
      }

      delete[] this->itemMemory;
#if !defined(NDEBUG)
      this->itemMemory = nullptr;
#endif
    }
    
    /// <summary>Tries to append the specified element to the queue</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(const TElement &element) {
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_consume // consume: math below carries dependency
      );
      std::size_t nextWriteIndex = (safeWriteIndex + 1) % this->capacity;

      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_acquire // consume: if() below carries dependency
      );
      if(nextWriteIndex == safeReadIndex) {
        return false; // Queue was full
      } else {
        TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeWriteIndex;
        new(writeAddress) TElement(element);
        this->writeIndex.store(nextWriteIndex, std::memory_order_release);
        return true; // Item was appended
      }
    }

    /// <summary>Tries to move-append the specified element to the queue</summary>
    /// <param name="element">Element that will be move-appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(TElement &&element) {
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_consume // consume: math below carries dependency
      );
      std::size_t nextWriteIndex = (safeWriteIndex + 1) % this->capacity;

      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_acquire // consume: if() below carries dependency
      );
      if(nextWriteIndex == safeReadIndex) {
        return false; // Queue was full
      } else {
        TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeWriteIndex;
        new(writeAddress) TElement(std::move(element)); // with move semantics
        this->writeIndex.store(nextWriteIndex, std::memory_order_release);
        return true; // Item was appended
      }
    }

    /// <summary>Tries to remove an element from the queue</summary>
    /// <param name="element">Element into which the queue's element will be placed</param>
    public: bool TryTake(TElement &element) {
      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_consume // consume: if() below carries dependency
      );
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_acquire // consume: if() below carries dependency
      );
      if(safeReadIndex == safeWriteIndex) {
        return false; // Queue was empty
      } else {
        TElement *readAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeReadIndex;
        element = std::move(*readAddress); // Does move assignment if available, otherwise copy
        readAddress->~TElement(); // Even after move, destructor would still have to be called
        this->readIndex.store((safeReadIndex + 1) % this->capacity, std::memory_order_release);
        return true; // Item was read
      }
    }

    /// <summary>Returns the maximum number of items the queue can hold</summary>
    /// <returns>The maximum number of items the queue can hold</returns>
    public: std::size_t GetCapacity() const { return this->capacity - 1U; }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: const std::size_t capacity;
    /// <summary>Index from which the next item will be read</summary>
    private: std::atomic<std::size_t> readIndex;
    /// <summary>Index at which the most recently written item is stored</summary>
    /// <remarks>
    ///   Notice that contrary to usual practice, this does not point one past the last
    ///   item (i.e. position for next to the written), but is the index of the last item
    ///   that has been stored in the queue. The lock-free synchronization is easier this way.
    /// </remarks>
    private: std::atomic<std::size_t> writeIndex;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    private: std::uint8_t *itemMemory;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
