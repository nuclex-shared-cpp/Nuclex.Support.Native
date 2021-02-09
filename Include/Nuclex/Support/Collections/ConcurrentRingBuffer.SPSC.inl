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

    /// <summary>Counts the items in the queue</summary>
    /// <returns>The number of items stored in the queue at the time of the call</returns>
    /// <remarks>
    ///   <para>
    ///     This method may be called from both the consuming and the producing thread.
    ///   </para>
    ///   <para>
    ///     So long as you conform to the single producer, single consumer requirement, you
    ///     can use this method a) in the consumer thread to find the number of items that
    ///     will <em>at least</em> be available via the <see cref="TryPop" /> method or
    ///     b) in the producer thread to find the amount of free space that will <em>at
    ///     least</em> be available to fill via the <see cref="Append" /> method.
    ///   </para>
    ///   <para>
    ///     If you call this method from an unrelated thread, there's a low but non-zero
    ///     chance that it will return complete garbage. So don't do that.
    ///   </para>
    /// </remarks>
    public: std::size_t Count() const {

      // Obtain read and write indices. Order is important here since this method can be
      // called from both the consumer and the producer thread.
      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_consume // consume: if() below carries dependency
      );
      // If this method is called from a third thread, it is possible that between the two
      // loads, both consumer and producer thread do work, moving the read index to
      // a position that has no more relation to the read index we just loaded.
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_acquire // acquire: access must happen after readIndex
      );

      // Are the items in the queue fragmented?
      if(safeWriteIndex < safeReadIndex) {
        return (this->capacity - safeReadIndex + safeWriteIndex + 1);
      } else { // Items are linear
        return (safeWriteIndex - safeReadIndex);
      }

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
      if(likely(nextWriteIndex != safeReadIndex)) {
        TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeWriteIndex;
        new(writeAddress) TElement(element);
        this->writeIndex.store(nextWriteIndex, std::memory_order_release);
        return true; // Item was appended
      } else {
        return false; // Queue was full
      }
    }

#if 0 // untested

    /// <summary>Tries to append multiple elements to the queue</summary>
    /// <param name="first">First of a list of elements that will be appended</param>
    /// <param name="count">Number of elements available from the list</param>
    /// <returns>The number of items that have been appended to the queue</returns>
    public: std::size_t TryAppend(const TElement *first, std::size_t count) {
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_consume // consume: if() below carries dependency
      );
      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_consume // consume: if() below carries dependency
      );

      // Is the used space fragmented? Then the free space is linear, easiest case!
      if(safeWriteIndex < safeReadIndex) {

        count = std::min(count, safeReadIndex - (safeWriteIndex + 1));
        if(likely(count >= 1)) {
          std::copy_n(first, count, this->itemMemory + safeWriteIndex + 1);
          this->writeIndex.store(
            safeWriteIndex + count, std::memory_order::memory_order_release
          );
        }
        return count;

      } else { // Used space was linear, so free sapce might be fragmented...

        std::size_t availableItemCount = this->capacity - (safeWriteIndex + 1);
        if(likely(availableItemCount >= count)) {
          std::copy_n(first, count, this->itemMemory + safeWriteIndex + 1);
          this->writeIndex.store(
            safeWriteIndex + count, std::memory_order::memory_order_release
          );
          return count;
        } else {
          // Write the first fragment at the end of the buffer
          if(availableItemCount >= 1) {
            std::copy_n(first, availableItemCount, this->itemMemory + safeWriteIndex + 1);
            this->writeIndex.store(
              this->capacity - 1, std::memory_order::memory_order_release
            ); // Intermediate store, allows reading threads to begin reading early
          }

          // Write the second fragment at the start of the buffer
          count = std::min(safeReadIndex, count - availableItemCount);
          if(likely(count >= 1)) { // Buffer may happen to be full!
            std::copy_n(first + availableItemCount, count, this->itemMemory);
            this->writeIndex.store(
              count - 1, std::memory_order::memory_order_release
            );
          }

          return (availableItemCount + count);
        }

      }
    }

#endif

    /// <summary>Tries to move-append the specified element to the queue</summary>
    /// <param name="element">Element that will be move-appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryShove(TElement &&element) {
      std::size_t safeWriteIndex = this->writeIndex.load(
        std::memory_order::memory_order_consume // consume: math below carries dependency
      );
      std::size_t nextWriteIndex = (safeWriteIndex + 1) % this->capacity;

      std::size_t safeReadIndex = this->readIndex.load(
        std::memory_order::memory_order_acquire // consume: if() below carries dependency
      );
      if(likely(nextWriteIndex != safeReadIndex)) {
        TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeWriteIndex;
        new(writeAddress) TElement(std::move(element)); // with move semantics
        this->writeIndex.store(nextWriteIndex, std::memory_order_release);
        return true; // Item was appended
      } else {
        return false; // Queue was full
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
