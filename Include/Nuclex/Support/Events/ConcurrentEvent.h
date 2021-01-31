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

#ifndef NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
#define NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Delegate.h"

#include <atomic>
#include <algorithm> // for std::copy_n()
#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  // Prototype, required for variable argument template
  template<typename> class ConcurrentEvent;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a list of subscribers that receive callbacks when the event fires</summary>
  /// <typeparam name="TResult">Type that will be returned from the method</typeparam>
  /// <typeparam name="TArguments">Types of the arguments accepted by the callback</typeparam>
  template<typename TResult, typename... TArguments>
  class ConcurrentEvent<TResult(TArguments...)> {

    /// <summary>Number of subscribers the event can subscribe withou allocating memory</summary>
    /// <remarks>
    ///   To reduce complexity, this value is baked in and not a template argument. It is
    ///   the number of subscriber slots that are baked into the event, enabling it to handle
    ///   a small number of subscribers without allocating heap memory. Each slot takes the size
    ///   of a delegate, 64 bits on a 32 bit system or 128 bits on a 64 bit system.
    /// </remarks>
    private: const static std::size_t BuiltInSubscriberCount = 2;

    /// <summary>Type of value that will be returned by the delegate</summary>
    public: typedef TResult ResultType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    public: typedef TResult CallType(TArguments...);
    /// <summary>Type of delegate used to call the event's subscribers</summary>
    public: typedef Delegate<TResult(TArguments...)> DelegateType;


    // Plan:
    //
    // - Counter
    // - If counter after increment >= 2, check heap
    // - 2 stack-alloc'd subscribers
    // - (status: 0 free, 1 building, 2 subscribed) ?
    // - Array or linked list of subscribes
    //
    // Reclamation:
    //
    // - Call counter?
    // - At decrement, CAS with 0xff special value if it is 1
    //   - If special value was applied, process free list
    //   - Otherwise do normal decrement (may miss opportunities to free)
    //
    //
    // Reclamation 2:
    //
    // - Opportunistic mutex?
    // - At decrement, try_lock() and if succeeds, process free list
    // - 
    //
    // Crazy 1:
    //
    // - First built-in subscriber's 'Next' link is the free list
    // - Second built-in subscriber's 'Next' link is the additional subscriber list




    /// <summary>Information about subscribers if the list is moved to the heap</summary>
    private: struct Subscriber {

      /// <summary>Delegate through which the subscriber will be called</summary>
      public: DelegateType Callback;
      /// <summary>Link to the next subscriber to the event</summary>
      public: std::atomic<Subscriber *> Next;

    };


    /// <summary>First subscriber in singly-linked list of subscribers</summary>
    private: Subscriber *first;
    /// <summary>Status of the built-in subscibrer slots</summary>
    /// <remarks>
    ///   0: free, 1: occupied, 2: released (will be freed when call count reaches 0)
    /// </remarks>
    private: std::atomic<std::uint_fast8_t> builtInStatus[BuiltInSubscriberCount];
    private: Subscriber builtInSubscriber[BuiltInSubscriberCount];



  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_EVENT_H
