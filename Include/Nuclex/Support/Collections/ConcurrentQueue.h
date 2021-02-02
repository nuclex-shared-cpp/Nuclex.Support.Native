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

    /// <summary>Initializeda new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentQueue(std::size_t capacity) :
      capacity(capacity),
      readIndex(0),
      writeIndex(0),
      itemMemory(new std::uint8_t(sizeof(TElement[capacity]))) {}
    
    public: ~ConcurrentQueue() {
      // TODO: Destroy items that are currently filled
      delete[] this->itemMemory;
    }
    
    public: void Append(const TElement &element) {
    }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: std::size_t capacity;
    /// <summary>Index from which the next item will be read</summary>
    private: std::atomic<std::size_t> readIndex;
    /// <summary>Index at which the next item will be written</summary>
    private: std::atomic<std::size_t> writeIndex;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    private: std::uint8_t *itemMemory;
    //private: std::atomic<std::size_t> 



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
