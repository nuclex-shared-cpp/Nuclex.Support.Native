#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#include "../Config.h"

#if defined(NUCLEX_SUPPORT_WIN32) || defined(NUCLEX_SUPPORT_WINRT)

#include <concurrent_queue.h>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Queue that can safely be used from multiple threads</summary>
  template<typename TElement>
  class ConcurrentQueue {

    /// <summary>Initializes a new concurrent queue</summary>
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

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#else

  #error Please implement ConcurrentQueue for your compiler

#endif

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTQUEUE_H
