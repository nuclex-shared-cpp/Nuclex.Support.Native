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

#include <cstdint> // for std::uint8_t
#include <atomic> // for std::atomic
#include <memory> // for std::shared_ptr
#include <mutex> // for std::mutex

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
    // Even firing is still assumed to be much more frequent than subscribe/unsubscribe.
    //
    // - Have a list of subscribers and another list that is used to add/remove stuff
    //   Problem: how to switch out the broadcast and edit queue instances?
    //
    // - It all comes back to needing an access counter for the broadcast queue.
    //
    // Alternative
    //
    // - Have an on-demand mutex only for editing and a shared_ptr to the list that
    //   broadcast threads simply acquire (and the promise that the list is immutable)
    //
    //   struct BroadcastQueue {
    //     Delegate *Subscribers;    // Plain array of subscribers? Singly linked list instead? Deque-like?
    //     size_t SubscriberCount;   // Number of Subscribers in the list
    //   };
    //
    //   // Will be reassigned if queue is edited
    //   std::shared_ptr<BroadcastQueue> broadcastQueue;
    //
    //   // Problem: how to synchronize edits? What if I don't want a permanent mutex?
    //   // Can I C-A-S a shared_ptr to a mutex?
    //   std::atomic<std::shared_ptr<std::mutex>> editMutex;
    //
    //   // If it works, how costly is it to construct a mutex?
    //   // If pthreads, it's just a bunch of ints. In Windows, is there a kernel mode switch?


    public: ConcurrentEvent() {
    }


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
      std::shared_ptr<std::mutex> currentEditMutex = std::atomic_load_explicit(
        &this->editMutex, std::memory_order::memory_order_consume // if() carries dependency
      );
      while(!currentEditMutex) {
        std::atomic_compare_exchange_strong(
          &this->editMutex, // pointer that will be replaced
          &currentEditMutex, // expected prior mutex, receives current on failure
          std::make_shared<std::mutex>() // new mutex to assign if previous was empty
        );
      }

      // At this point, we either grabbed the existing mutex or made our new mutex available
      // for other threads attempting to edit the subscriber list.
      //
      // Since we C-A-S to kill the mutex only after all work is done,
      // we sidestep a possible race condition where multiple mutexes may exist

      // Build a new broadcast list with the new subscriber appended to the end
      {
        std::lock_guard<std::mutex> editMutexLock(currentEditMutex);

        //assert(!!this->subscribers && u8"Subscriber queue is always present");
        if(!this->subscribers) {
          std::shared_ptr<BroadcastQueue> newQueue = std::make_shared<BroadcastQueue>(1);
        } else {
          const BroadcastQueue &currentQueue = *this->subscribers.get();

          std::shared_ptr<BroadcastQueue> newQueue = std::make_shared<BroadcastQueue>(
            currentQueue.SubscriberCount + 1
          );
          // TODO: Copy queue

        }

      }
    }

    #pragma region struct BroadcastQueue

    /// <summary>Queue of subscribers to which the event will be broadcast</summary>
    private: struct BroadcastQueue {

      public: BroadcastQueue(std::size_t count) :
        Subscribers(allocateUninitializedDelegates(count)),
        SubscriberCount(count) {}

      public: ~BroadcastQueue() {
        freeDelegatesWithoutDestructor(this->Subscribers);
      }

      BroadcastQueue(const BroadcastQueue &other) = delete;
      BroadcastQueue(BroadcastQueue &&other) = delete;
      void operator =(const BroadcastQueue &other) = delete;
      void operator =(BroadcastQueue &&other) = delete;

      private: static DelegateType *allocateUninitializedDelegates(std::size_t count) {
        return reinterpret_cast<DelegateType *>(
          new std::uint8_t *[sizeof(DelegateType[2]) * count / 2]
        );
      }

      private: static void freeDelegatesWithoutDestructor(DelegateType *delegates) {
        delete[] reinterpret_cast<std::uint8_t *>(delegates);
      }

      /// <summary>Plain array of all subscribers to which the event is broadcast</summary>
      public: DelegateType *Subscribers;
      /// <summary>Number of subscribers stored in the array</summary>
      public: std::size_t SubscriberCount;

    };

    #pragma endregion // struct BroadcastQueue

    #pragma region struct SharedMutex

    /// <summary>Queue of subscribers to which the event will be broadcast</summary>
    private: struct SharedMutex {

      /// <summary>Initializes a new shared mutex</summary>
      public: SharedMutex() :
        Mutex(),
        ReferenceCount(1) {}

      /// <summary>Mutex that is shared between multiple owners</summary>
      public: std::mutex Mutex;
      /// <summary>Number of references to this instance of shared mutex<</summary>
      public: std::atomic<std::size_t> ReferenceCount;

    };

    #pragma endregion // struct SharedMutex

    /// <summary>Stores the current subscribers to the event</summary>
    public: /* atomic */ std::shared_ptr<const BroadcastQueue> subscribers;
    /// <summary>Will be present while subscriptions/unsubscriptions happen</summary>
    public: /* atomic */ std::shared_ptr<std::mutex> editMutex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
