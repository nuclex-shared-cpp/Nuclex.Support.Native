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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H

#include "Nuclex/Support/Collections/ConcurrentCollection.h"

#include <atomic> // for std::atomic
#include <cstdint> // for std::uint8_t

// CHECK: Rename this to ConcurrentRingBuffer? or something else?
//
// RingBuffer (single-threaded) currently is unbounded. People may expect a fixed-size
// buffer when seeing the name ring buffer (or anything ending in buffer).
//
// Queue does not indicate boundedness. So perhaps I have written
//   - A RingQueue
//   - A ShiftQueue
//   - A ConcurrentRingBuffer
//
// Downside: many papers and libraries talk about "bounded queues" and mean exactly
// what I've implemented here...
//

//#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"
//#include "Nuclex/Support/Collections/MoodyCamel/readerwriterqueue.h"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  // Forward declaration for clarity and to move all specializations into separate files
  template<
    typename TElement,
    ConcurrentAccessBehavior accessBehavior = (
      ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentRingBuffer;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#include "ConcurrentRingBuffer.SPSC.inl"
//#include "ConcurrentRingBuffer.MPSC.inl"
//#include "ConcurrentRingBuffer.MPMC.inl"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size list that can safely be used from multiple threads</summary>
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::MultipleProducersSingleConsumer> {

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity + 1),
      readIndex(0),
      writeIndex(0),
      occupiedIndex(0),
      finishedWriteCount(0),
      itemMemory(new std::uint8_t(sizeof(TElement[2]) * (capacity + 1) / 2U)) {}
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentRingBuffer() {
      if constexpr(!std::is_trivially_destructible<TElement>::value) {
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_acquire
        );
        std::size_t safeWriteIndex = this->writeIndex.load(
          std::memory_order::memory_order_acquire
        );
        while(safeReadIndex != safeWriteIndex) {
          reinterpret_cast<TElement *>(this->itemMemory)[safeReadIndex].~TElement();
          safeReadIndex = (safeReadIndex + 1) % this->capacity;
        }
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
      std::size_t nextOccupiedIndex;
      for(;;) {
        std::size_t safeOccupiedIndex = this->occupiedIndex.load(
          std::memory_order::memory_order_acquire
        );
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_acquire
        );

        nextOccupiedIndex = (safeOccupiedIndex + 1) % this->capacity;
        if(nextOccupiedIndex == safeReadIndex) {
          return false; // Queue is full
        } else {
          /*
          bool wasReplaced = this->occupiedIndex.compare_exchange_weak(
            safeOccupiedIndex, nextOccupiedIndex, std::memory_order_release
          );
          if(wasReplaced) {
            break;
          }
          */
        }
      }

      return false;
    }

    private: bool tryOccupySlot(std::size_t &nextOccupiedIndex) {
      for(;;) {
        std::size_t safeOccupiedIndex = this->occupiedIndex.load(
          std::memory_order::memory_order_acquire
        );
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_acquire
        );

        nextOccupiedIndex = (safeOccupiedIndex + 1) % this->capacity;
        if(nextOccupiedIndex == safeReadIndex) {
          return false; // Queue is full
        } else {
          /*
          bool wasReplaced = this->occupiedIndex.compare_exchange_weak(
            safeOccupiedIndex, nextOccupiedIndex, std::memory_order_release
          );
          if(wasReplaced) {
            return true;
          }
          */
        }

        // this->occupiedIndex has changed while we were working, try again...
      }
    }

    /// <summary>Tries to move-append the specified element to the queue</summary>
    /// <param name="element">Element that will be move-appended to the queue</param>
    /// <returns>True if the element was appended, false if the queue had no space left</returns>
    public: bool TryAppend(TElement &&element) {

      // Try to reserve a slot. If the queue is full, the value will be zero (or less,
      // when contested).
      int remainingSlotCount = this->freeSlotCount.fetch_sub(1, std::memory_order_relaxed);
      if(remainingSlotCount < 1) {
        this->freeSlotCount.fetch_add(1, std::memory_order_relaxed);
        return false;
      }

      // If we reach this spot, we know there was at least 1 slot free in the queue and we
      // just captured it. Take one from the occupied index list.
      std::size_t safeWriteIndex = this->writeIndex.load(std::memory_order_acquire);
      std::size_t safeOccupiedIndex = this->occupiedIndex.fetch_add(1, std::memory_order_acquire);
      safeOccupiedIndex %= this->capacity; // Uh-oh, safeOccupiedIndex grows without bounds here...

      // Place the item in the buffer
      TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeOccupiedIndex;
      new(writeAddress) TElement(std::move(element));

      // Increment the number of completed writes.
      std::size_t safeFinishedWriteCount = this->finishedWriteCount.fetch_add(
        1, std::memory_order_release
      );

      // If all writes up to the new occupied index have finished at this point (meaning no
      // uninitialized or under construction gaps exist between the visible write index and
      // the occupied index we just wrote to, we can expose all writes up to that point
      if(safeWriteIndex + safeFinishedWriteCount == safeOccupiedIndex) {
        this->finishedWriteCount.fetch_sub(safeFinishedWriteCount, std::memory_order_relaxed);
        this->writeIndex.fetch_add(safeFinishedWriteCount, std::memory_order_relaxed);
      }

    }

    /// <summary>Tries to remove an element from the queue</summary>
    /// <param name="element">Element into which the queue's element will be placed</param>
    public: bool TryTake(TElement &element) {



      std::size_t safeReadIndex = this->readIndex.load(std::memory_order::memory_order_acquire);
      std::size_t safeWriteIndex = this->writeIndex.load(std::memory_order::memory_order_acquire);

      if(safeReadIndex == safeWriteIndex) {
        return false; // Queue is empty
      } else {
        TElement *readAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeReadIndex;
        element = std::move(*readAddress);
        readAddress->~TElement();
        this->readIndex.store((safeReadIndex + 1) % this->capacity, std::memory_order_release);
        return true; // Item was read
      }
    }

    /// <summary>Performs the modulo operation, but returns 0..divisor-1</summary>
    /// <param name="value">Value for which the positive modulo will be calculated</param>
    /// <param name="divisor">Divisor of which the remainder will be calculated</param>
    /// <returns>The positive division remainder of the specified value</returns>
    /// <remarks>
    ///   There are various tricks to achieve this without branching, but they're all slower.
    ///   Reason: x86, amd64 and ARM CPUs have conditional move instructions, allowing cases
    ///   like this one to execute without branching at the machine code level.
    /// </remarks>
    private: static int positiveModulo(int value, int divisor) {
      value %= divisor;
      if(value < 0) {
        return value + divisor;
      } else {
        return value;
      }
    }

    /// <summary>Returns the maximum number of items the queue can hold</summary>
    /// <returns>The maximum number of items the queue can hold</returns>
    public: std::size_t GetCapacity() const { return this->capacity - 1U; }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: const std::size_t capacity;
    /// <summary>Number of free slots the queue can store elements in</summary>
    /// <remarks>
    ///   This allows the <see cref="Append" /> method to know whether a slot will be free
    ///   after the current write index, eliminating the whole C-A-S loop. While reserving,
    ///   the value will be blindly decremented, then checked and - if negative - incremented
    ///   back up.
    /// </remarks>
    private: std::atomic<int> freeSlotCount;
    /// <summary>Index from which the next item will be read</summary>
    private: std::atomic<int> readIndex;
    /// <summary>Index at which the most recently written item is stored</summary>
    /// <remarks>
    ///   Notice that contrary to usual practice, this does not point one past the last
    ///   item (i.e. position for next to the written), but is the index of the last item
    ///   that has been stored in the queue. The lock-free synchronization is easier this way.
    /// </remarks>
    private: std::atomic<int> writeIndex;
    /// <summary>Like write index, but for the next item to be filled</summary>
    private: std::atomic<int> occupiedIndex;
    /// <summary>Number of pending writes that have been finished</summary>
    /// <remarks>
    ///   When the <see cref="occupiedIndex" /> moves ahead of the write index, other threads
    ///   may also begin writing to the queue (and could finish earlier than prior ones).
    ///   Each thread that finishes a write will increment the finished write count.
    ///   If <see cref="writeIndex" /> plus <see cref="finishedWriteCount" /> equals
    ///   <see cref="occupiedIndex" /> then <see cref="writeIndex" /> can be moved ahead
    ///   without exposing items under construction. The only danger is that when the queue
    ///   is highly contested, this state may (in the worst case) only be achieved once
    ///   the queue is full.
    /// </remarks>
    private: std::atomic<std::size_t> finishedWriteCount;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    private: std::uint8_t *itemMemory;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Queue that can safely be used from multiple threads</summary>
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::MultipleProducersMultipleConsumers> {
/*
    /// <summary>Queue that is wrapped to provide all functionality</summary>
    private: moodycamel::ConcurrentQueue<TElement> wrappedQueue;
*/
  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H
