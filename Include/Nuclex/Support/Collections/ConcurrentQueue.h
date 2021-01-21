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

#include "Nuclex/Support/Config.h"

// Known implementations besides this one for reference:
//
// Libraries of Lock-Free data structures:
// https://github.com/rigtorp/awesome-lockfree
// https://github.com/mpoeter/xenium
// https://github.com/oneapi-src/oneTBB (Intel TBB under its new name)
// https://liblfds.org/ (<-- Public Domain!)
//
// Interesting implementations:
// https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++.htm
// https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++.htm
//
// Intel's implementation (curiously not that good in benchmarks):
// https://github.com/oneapi-src/oneTBB/blob/master/include/oneapi/tbb/concurrent_queue.h
//
// "Battle Tested" implementation:
// https://github.com/rigtorp/MPMCQueue

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>How a concurrent queue is being accessed</summary>
  /// <remarks>
  ///   There fewer threads need to access the queue, the faster our implementation
  ///   can be. This is used as a template parameter to decide implementation.
  /// </remarks>
  enum class ConcurrentQueueAccessBehavior {

    /// <summary>
    ///   Only one thread is taking data and another, but only one, is producing it
    /// </summary>
    SingleProducerSingleConsumer,

    /// <summary>
    ///   Only one thread is taking data, but multiple threads are adding data
    /// </summary>
    MultipleProducersSingleConsumer,

    /// <summary>
    ///   Any number of threads is taking data and any number of threads is adding it
    /// </summary>
    MultipleProducersMultipleConsumers

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Queue that can safely be used from multiple threads</summary>
  template<
    typename TElement,
    ConcurrentQueueAccessBehavior accessBehavior = (
      ConcurrentQueueAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentQueue {
#if 0
    /// <summary>Initializes an empty concurrent queue</summary>
    public: ConcurrentQueue() {}

    /// <summary>Destroys the concurrent queue</summary>
    public: ~ConcurrentQueue() {}

    /// <summary>Appends an element to the queue in a thread-safe manner</summary>
    /// <param name="element">Element that will be appended to the queue</param>
    public: void Append(const TElement &element) {
      this->wrappedQueue.push(element);
    }

    /// <summary>Tries to take an element from the queue</summary>
    /// <param name="element">Will receive the element taken from the queue</param>
    /// <returns>
    ///   True if an element was taken from the queue, false if the queue was empty
    /// </returns>
    public: bool TryPop(TElement &element) {
      return this->wrappedQueue.try_pop(element);
    }

    private: ConcurrentQueue(const ConcurrentQueue &);
    private: ConcurrentQueue &operator =(const ConcurrentQueue &);

    /// <summary>Concurrent queue this class is acting as an adapter for</summary>
    private: concurrency::concurrent_queue<TElement> wrappedQueue;
#endif
  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
