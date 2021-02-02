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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H

#include "Nuclex/Support/Collections/ConcurrentCollection.h"

#include <atomic> // for std::atomic
#include <cstdint> // for std::uint8_t

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
  class ConcurrentQueue;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size list that can safely be used from multiple threads</summary>
  template<typename TElement>
  class ConcurrentQueue<TElement, ConcurrentAccessBehavior::SingleProducerSingleConsumer> {

    /// <summary>Value indicating an invalid index for the next write</summary>
    private: static const std::size_t InvalidIndex = static_cast<std::size_t>(-1);

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentQueue(std::size_t capacity) :
      capacity(capacity + 1),
      readIndex(0),
      writeIndex(0),
      itemMemory(new std::uint8_t(sizeof(TElement[2]) * (capacity + 1) / 2U)) {}
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentQueue() {
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
      std::size_t safeWriteIndex = this->writeIndex.load(std::memory_order::memory_order_acquire);
      std::size_t safeReadIndex = this->readIndex.load(std::memory_order::memory_order_acquire);

      std::size_t nextWriteIndex = (safeWriteIndex + 1) % this->capacity;
      if(nextWriteIndex == safeReadIndex) {
        return false; // Queue is full
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
      std::size_t safeWriteIndex = this->writeIndex.load(std::memory_order::memory_order_acquire);
      std::size_t safeReadIndex = this->readIndex.load(std::memory_order::memory_order_acquire);

      std::size_t nextWriteIndex = (safeWriteIndex + 1) % this->capacity;
      if(nextWriteIndex == safeReadIndex) {
        return false; // Queue is full
      } else {
        TElement *writeAddress = reinterpret_cast<TElement *>(this->itemMemory) + safeWriteIndex;
        new(writeAddress) TElement(std::move(element));
        this->writeIndex.store(nextWriteIndex, std::memory_order_release);
        return true; // Item was appended
      }
    }

    public: bool TryTake(TElement &element) {
      return false;
      
    }

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

  /// <summary>Queue that can safely be used from multiple threads</summary>
  template<typename TElement>
  class ConcurrentQueue<TElement, ConcurrentAccessBehavior::MultipleProducersMultipleConsumers> {
/*
    /// <summary>Queue that is wrapped to provide all functionality</summary>
    private: moodycamel::ConcurrentQueue<TElement> wrappedQueue;
*/
  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
