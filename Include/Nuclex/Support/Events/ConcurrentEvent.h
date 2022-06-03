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
#include "Nuclex/Support/ScopeGuard.h"

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


    /// <summary>Initializes a new concurrent event</summary>
    public: ConcurrentEvent() = default;

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
      std::shared_ptr<SharedMutex> currentEditMutex = acquireMutex();
      ON_SCOPE_EXIT {
        releaseMutex(currentEditMutex);
      };
      {
        currentEditMutex->Mutex.lock();
        ON_SCOPE_EXIT {
          currentEditMutex->Mutex.unlock();
        };

        // Build a new broadcast list with the new subscriber appended to the end
        {
          std::shared_ptr<BroadcastQueue> newQueue;

          // We either have to create an entirely new list of subscribers or
          // clone an existing subscriber list while adding one entry to the end.
          if(!this->subscribers) {
            newQueue = std::make_shared<BroadcastQueue>(1);
            new(newQueue->Subscribers) DelegateType(delegate);
          } else { // Non-empty subscriber list present, create clone with extra entry
            std::size_t currentSubscriberCount = this->subscribers->SubscriberCount;
            newQueue = std::make_shared<BroadcastQueue>(currentSubscriberCount + 1);
            for(std::size_t index = 0; index < currentSubscriberCount; ++index) {
              new(newQueue->Subscribers + index) DelegateType(this->subscribers->Subscribers[index]);
            }
            new(newQueue->Subscribers + currentSubscriberCount) DelegateType(delegate);
          }

          // This might be a little more efficient if I drop the const BroadcastQueue stuff.
          // And isn't there an std::atomic_store for std::shared_ptr with move semantics?
          std::atomic_store(&this->subscribers, std::shared_ptr<const BroadcastQueue>(newQueue));
        } // Queue replacement beauty scope
      } // edit mutex lock scope
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

    /// <summary>Acquires the edit mutex held while editing the broadcast queue</summary>
    /// <returns>The current edit mutex</returns>
    /// <remarks>
    ///   This goes through some hoops to ensure the mutex only exists while the broadcast
    ///   queue is being edited, while also ensuring that if a mutex exist, only one exists
    ///   and is shared by all threads competing to edit the broadcast queue.
    /// </remarks>
    private: std::shared_ptr<SharedMutex> acquireMutex() {
      std::shared_ptr<SharedMutex> currentEditMutex = std::atomic_load_explicit(
        &this->editMutex, std::memory_order::memory_order_consume // if() carries dependency
      );
      std::shared_ptr<SharedMutex> newEditMutex; // Leave as nullptr until we know we need it
      for(;;) {

        // If we got a shared mutex instance, it may still be on the brink of being abandoned
        // (once the reference counter goes to 0, it must not be revived to avoid a race)
        if(unlikely(static_cast<bool>(currentEditMutex))) {
          std::size_t knownReferenceCount = currentEditMutex->ReferenceCount.load(
            std::memory_order::memory_order_consume
          );

          // Was the mutex just now still occupied by another thread? If so, attempt to
          // increment its reference count one further via a C-A-S operation.
          while(likely(knownReferenceCount >= 1)) {
            bool wasExchanged = currentEditMutex->ReferenceCount.compare_exchange_weak(
              knownReferenceCount, knownReferenceCount + 1
            );
            if(likely(wasExchanged)) {
              return currentEditMutex; // We managed to increment the reference count above 1
            }
          }

          // The shared mutex for which we got the reference count was about to be dropped
          currentEditMutex.reset(); // Assume the other thread has already dropped it by now
        }

        // Either there was no existing mutex or it was about to be dropped,
        // so attempt to insert our own, fresh mutex with a reference count of 1.
        if(!newEditMutex) {
          newEditMutex = std::make_shared<SharedMutex>();
        }
        bool wasExchanged = std::atomic_compare_exchange_strong(
          &this->editMutex, // pointer that will be replaced
          &currentEditMutex, // expected prior mutex, receives current on failure
          newEditMutex // new mutex to assign if previous was empty
        );
        if(likely(wasExchanged)) {
          return newEditMutex; // We got our new mutex in with 1 reference count held by us
        }
      }
    }

    /// <summary>Releases the shared mutex again, potentially dropping it entirely</summary>
    /// <param name="currentEditMutex">Shared mutex as returned by the acquire method</param>
    private: void releaseMutex(const std::shared_ptr<SharedMutex> &currentEditMutex) {
      std::size_t previousReferenceCount = currentEditMutex->ReferenceCount.fetch_sub(
        1, std::memory_order::memory_order_seq_cst
      );

      // If we just decremented the reference counter to zero, drop the shared mutex.
      // This would be a race condition for a naively implemented acquireMutex() method,
      // but we sidestep this by making acquireMutex() C-A-S the reference count for
      // the uptick operation and consider the whole whole shared mutex dead when seeing
      // it at a reference count of zero.
      if(likely(previousReferenceCount == 1)) {
        std::atomic_store(&this->editMutex, std::shared_ptr<SharedMutex>());
      }
    }

    /// <summary>Stores the current subscribers to the event</summary>
    public: /* atomic */ std::shared_ptr<const BroadcastQueue> subscribers;
    /// <summary>Will be present while subscriptions/unsubscriptions happen</summary>
    public: /* atomic */ std::shared_ptr<SharedMutex> editMutex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_CONCURRENTEVENT_H
