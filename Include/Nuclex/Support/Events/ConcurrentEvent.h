#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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
  /// <typeparam name="TResult">Type of results the callbacks will return</typeparam>
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

    //
    // -----------------------------------
    //
    // Possible:
    //
    //  Hybrid Mutex/Lock-Wait-Free Broadcast Queue
    //
    //  - Singly linked list of subscribers
    //  - Counter that is incremented while firing
    //
    //  - Subscribe()/Unsubscribe() enter Mutex
    //  - Clone complete subscriber list with desired modification
    //  - Exchange subscriber list start node
    //  - Leave Mutex
    //
    //  - When publisher sees counter go to 0, clean up exchanged (orphaned) lists
    //    (tricky, should not enter mutex)
    //
    //  Possible improvement
    //
    //  - Like above, but Subscribe()/Unsubscribe() check counter
    //  - If counter is zero at end of Subscribe()/Unsubscribe(), then no
    //    broadcast is currently active and exchanged list can be deleted immediately
    //
    //  Another improvement
    //
    //  - If counter is not zero at decrement after Unsubscribe(), CAS-append to
    //    opportunistic-free list
    //  - Broadcast will check opportunistic-free list when decrement causes counter
    //    to hit zero.
    //
    //  - PROBLEM: How to ensure that between counter hitting zero and CASing the free
    //    list, the free list isn't modified by a concurrent Susbcribe()/Unsubscribe()?
    //  - FIX?: Queue of queues where the broadcast list has its own counter and
    //          Unsubscribe() appends it to a queue of queues to be opportunistically freed?
    //
    //  Alternative improvement
    //
    //  - Subscribe()/Unsubscribe() simply enter a busy spin when the counter is not zero.
    //  - SHITTY: Who says an event can't spend seconds per broadcast receiver? Would then
    //            block the Subscribe()/Unsubscribe() caller very long and cause 100% CPU.
    //
    //  Trick
    //
    //  - Is adding by prepending generally safe?
    //    I see no circumstance in which that would become a problem, only removal,
    //    so long as the notification order is arbitrary
    //
    // -------------------------
    //
    // Switcheroo:
    //
    // Have a list of subscribers and another list that .
    //
    //

    /// <summary>Subscribes the specified free function to the event</summary>
    /// <typeparam name="TMethod">Free function that will be subscribed</typeparam>
    public: template<TResult(*TMethod)(TArguments...)>
    void Subscribe() {
      Subscribe(DelegateType::template Create<TMethod>());
    }

    /// <summary>Subscribes the specified object method to the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Object method that will be subscribed to the event</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    void Subscribe(TClass *instance) {
      Subscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Subscribes the specified const object method to the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Object method that will be subscribed to the event</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...) const>
    void Subscribe(const TClass *instance) {
      Subscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Subscribes the specified delegate to the event</summary>
    /// <param name="delegate">Delegate that will be subscribed</param>
    public: void Subscribe(const DelegateType &delegate) {
      this->accessCount.fetch_add(1, std::memory_order::memory_order_release);
      SubscriberQueue subscriberQueue = this->subscribers.load(
        std::memory_order::memory_order_acquire
      );

      std::size_t requiredByteCount = sizeof(SubscriberQueue);


    }

    #pragma region struct Subscriber

    /// <summary>Linked list node storing a subscriber callback for the event</summary>
    private: struct Subscriber {

      /// <summary>Delegate through which the subscriber will be called</summary>
      public: DelegateType Callback;
      /// <summary>Link to the next subscriber to the event</summary>
      public: Subscriber *Next;

    };

    #pragma endregion // struct Subscriber

    #pragma region struct SubscriberQueue

    /// <summary>Queue of subscribes with access counter for opportunistic free</summary>
    private: struct SubscriberQueue {

      /// <summary>First subscriber in the queue (in a singly linked list)</summary>
      public: Subscriber *First;
      /// <summary>Next queue if this queue is inthe opportunistic free set</summary>
      public: std::atomic<SubscriberQueue *> Next;

    };

    #pragma endregion // struct SubscriberQueue

    /// <summary>Number of active accessors to any subscriber list</summary>
    private: std::atomic<std::size_t> accessCount;

    /// <summary>Current subscribers to the event that will receive broadcasts</summary>
    private: std::atomic<SubscriberQueue *> subscribers;
    /// <summary>Older subscriber lists to deallocate when ceasing access</summary>
    private: std::atomic<SubscriberQueue *> previousSubscribers;


  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_EVENT_H
