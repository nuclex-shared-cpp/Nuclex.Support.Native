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
#include <exception> // for std::terminate()
#include <algorithm> // for std::copy_n()

// CHECK: Rename this to ConcurrentRingBuffer? or something else?
//
// I decided that "Buffers" are fixed-size (bounded) and "Queues" are not
//
// RingBuffer (single-threaded) currently is unbounded. People may expect a fixed-size
// buffer when seeing the name ring buffer (or anything ending in buffer).
//
// Queue does not indicate boundedness. So perhaps I have created:
//   - A RingQueue
//   - A ShiftQueue
//   - A ConcurrentRingBuffer
//
// Downside: many papers and libraries talk about "bounded queues" and mean exactly
// what I've implemented here...
//

// CHECK: Was the rename a shitty idea? Maybe Buffers = Batch operations, Queue = individual?
//
// In this case, rename everything back to:
//   - RingBuffer
//   - ShiftBuffer
//   - ConcurrentRingQueue
//   - 


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
#include "ConcurrentRingBuffer.MPSC.inl"
//#include "ConcurrentRingBuffer.MPMC.inl"

namespace Nuclex { namespace Support { namespace Collections {

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
