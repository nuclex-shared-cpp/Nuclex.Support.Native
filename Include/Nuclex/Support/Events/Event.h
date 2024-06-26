#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

#ifndef NUCLEX_SUPPORT_EVENTS_EVENT_H
#define NUCLEX_SUPPORT_EVENTS_EVENT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Delegate.h"

#include <algorithm> // for std::copy_n()
#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  // Prototype, required for variable argument template
  template<typename> class Event;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a list of subscribers that receive callbacks when the event fires</summary>
  /// <typeparam name="TResult">Type of results the callbacks will return</typeparam>
  /// <typeparam name="TArguments">Types of the arguments accepted by the callback</typeparam>
  /// <remarks>
  ///   <para>
  ///     This is the signal part of a standard signal/slot implementation. The name has been
  ///     chosen because <code>std::signal</code> already defines the term 'signal' for
  ///     an entirely thing and the term 'event' is the second most common term for this kind
  ///     of system.
  ///   </para>
  ///   <para>
  ///     The design makes a few assumptions on the usage patterns it optimizes for. It assumes
  ///     that events typically have a very small number of subscribers and that events should be
  ///     as lean as possible (i.e. rather than expose a single big multi-purpose notification,
  ///     classes would expose multiple granular events to notify about different things).
  ///     It also assumes that firing will happen much more often than subscribing/unsubscribing,
  ///     and subscribing is given slightly more performance priority than unsubscribing.
  ///   </para>
  ///   <para>
  ///     This variant of the event class is not thread safe. The order in which subscribers
  ///     are notified is not defined and may change even between individual calls. Subscribers
  ///     are allowed to unsubscribe themselves during an event call, but not others. Adding
  ///     new event subscriptions from within a call is supported, too.
  ///   </para>
  ///   <para>
  ///     An event should be equivalent in size to 5 pointers (depending on the
  ///     value of the <see cref="BuiltInSubscriberCount" /> constant))
  ///   </para>
  ///   <para>
  ///     Usage example:
  ///   </para>
  ///   <para>
  ///     <code>
  ///       int Dummy(int first, std::string second) { return 123; }
  ///
  ///       class Mock {
  ///         public: int Dummy(int first, std::string second) { return 456; }
  ///       };
  ///
  ///       int main() {
  ///         typedef Event&lt;int(int foo, std::string bar)&gt; FooBarEvent;
  ///
  ///         FooBarEvent test;
  ///
  ///         // Subscribe the dummy function
  ///         test.Subscribe&lt;Dummy&gt;();
  ///
  ///         // Subscribe an object method
  ///         Mock myMock;
  ///         test.Subscribe&lt;Mock, &Mock::Dummy&gt;(&amp;myMock);
  ///
  ///         // Fire the event
  ///         std::vector&lt;int&gt; returnedValues = test(123, u8"Hello");
  ///
  ///         // Fire the event again but don't collect returned values
  ///         test.Emit(123, u8"Hello");
  ///       }
  ///     </code>
  ///   </para>
  ///   <para>
  ///     Cheat sheet
  ///   </para>
  ///   <para>
  ///     🛈 Optimized for granular events (many event instances w/few subscribers)<br />
  ///     🛈 Optimized for fast broadcast performance over subscribe/unsubscribe<br />
  ///     🛈 No allocations up to <see cref="BuiltInSubscriberCount" /> subscribers<br />
  ///     ⚫ Can optionally collect return values from all event callbacks<br />
  ///     ⚫ New subscribers can be added freely even during event broadcast<br />
  ///     ⚫ Subscribers can unsubscribe themselves even from within event callback<br />
  ///     🛇 UNDEFINED BEHAVIOR on unsubscribing any other than self from within callback<br />
  ///     ⚫ For single-threaded use (publishers and subscribers share a single thread)<br />
  ///     🛇 UNDEFINED BEHAVIOR when accessed from multiple threads<br />
  ///        -> Multi-threaded broadcast is okay if no subscribe/unsubscribe happens
  ///        (i.e. subscribe phase, then threads run, threads end, then unsubscribe phase)<br />
  ///     🛇 Lambda expressions can not be subscribers<br />
  ///        (adds huge runtime costs, see std::function, would have no way to unsubscribe)<br />
  ///   </para>
  ///   <para>
  ///     If these restrictions are too much, consider <see cref="ConcurrentEvent" />, in which
  ///     basically anything goes for a small price in performance.
  ///   </para>
  /// </remarks>
  template<typename TResult, typename... TArguments>
  class Event<TResult(TArguments...)> {

    /// <summary>Number of subscribers the event can handle without allocating memory</summary>
    /// <remarks>
    ///   To reduce complexity, this value is baked in and not a template argument. It is
    ///   the number of subscriber slots that are baked into the event, enabling it to handle
    ///   a small number of subscribers without allocating heap memory. Each slot takes the size
    ///   of a delegate, 8 bytes on a 32 bit system or 16 bytes on a 64 bit system. If more
    ///   subscribers enlist, the event is forced to allocate memory on the heap.
    /// </remarks>
    private: const static std::size_t BuiltInSubscriberCount = 2;

    /// <summary>Type of value that will be returned by the delegate</summary>
    public: typedef TResult ResultType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    public: typedef TResult CallType(TArguments...);
    /// <summary>Type of delegate used to call the event's subscribers</summary>
    public: typedef Delegate<TResult(TArguments...)> DelegateType;

    /// <summary>List of results returned by subscribers</summary>
    /// <remarks>
    ///   Having an std::vector&lt;void&gt; anywhere, even in a SFINAE-disabled method,
    ///   will trigger deprecation compiler warnings on Microsoft compilers.
    ///   Consider this type to be an alias for std::vector&lt;TResult&gt; and nothing else.
    /// </remarks>
    private: typedef std::vector<
      typename std::conditional<std::is_void<TResult>::value, char, TResult>::type
    > ResultVectorType;

    /// <summary>Initializes a new event</summary>
    public: Event() :
      subscriberCount(0) {}

    /// <summary>Frees all memory used by the event</summary>
    public: ~Event() {
      if(this->subscriberCount > BuiltInSubscriberCount) {
        delete []this->heapMemory.Buffer;
      }
    }

    // TODO: Implement copy and move constructors + assignment operators

    /// <summary>Returns the current number of subscribers to the event</summary>
    /// <returns>The number of current subscribers</returns>
    public: std::size_t CountSubscribers() const {
      return this->subscriberCount;
    }

    /// <summary>Calls all subscribers of the event and collects their return values</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    /// <returns>An list of the values returned by the event subscribers</returns>
    /// <remarks>
    ///   This overload is enabled if the event signature returns anything other than 'void'.
    ///   The returned value is an std::vector&lt;TResult&gt; in all cases.
    /// </remarks>
    public: template<typename T = TResult>
    typename std::enable_if<
      !std::is_void<T>::value, ResultVectorType
    >::type operator()(TArguments&&... arguments) const {
      ResultVectorType results; // ResultVectorType is an alias for std::vector<TResult>
      results.reserve(this->subscriberCount);
      EmitAndCollect(std::back_inserter(results), std::forward<TArguments>(arguments)...);
      return results;
    }

    /// <summary>Calls all subscribers of the event</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    /// <remarks>
    ///   This overload is enabled if the event signature has the return type 'void'
    /// </remarks>
    public: template<typename T = TResult>
    typename std::enable_if<
      std::is_void<T>::value, void
    >::type operator()(TArguments&&... arguments) const {
      Emit(std::forward<TArguments>(arguments)...);
    }

    /// <summary>Calls all subscribers of the event and collects their return values</summary>
    /// <param name="results">
    ///   Output iterator into which the subscribers' return values will be written
    /// </param>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    public: template<typename TOutputIterator>
    void EmitAndCollect(TOutputIterator results, TArguments&&... arguments) const;

    /// <summary>Calls all subscribers of the event and discards their return values</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    public: void Emit(TArguments... arguments) const;

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
      if(this->subscriberCount < BuiltInSubscriberCount) {
        reinterpret_cast<DelegateType *>(this->stackMemory)[
          this->subscriberCount
        ] = delegate;
      } else {
        if(this->subscriberCount == BuiltInSubscriberCount) {
          convertFromStackToHeapAllocation();
        } else if(this->subscriberCount >= this->heapMemory.ReservedSubscriberCount) {
          growHeapAllocatedList();
        }

        reinterpret_cast<DelegateType *>(this->heapMemory.Buffer)[
          this->subscriberCount
        ] = delegate;
      }

      ++this->subscriberCount;
    }

    /// <summary>Unsubscribes the specified free function from the event</summary>
    /// <typeparam name="TMethod">
    ///   Free function that will be unsubscribed from the event
    /// </typeparam>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<TResult(*TMethod)(TArguments...)>
    bool Unsubscribe() {
      return Unsubscribe(DelegateType::template Create<TMethod>());
    }

    /// <summary>Unsubscribes the specified object method from the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">
    ///   Object method that will be unsubscribes from the event
    /// </typeparam>
    /// <param name="instance">Instance on which the object method was subscribed</param>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    bool Unsubscribe(TClass *instance) {
      return Unsubscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Unsubscribes the specified object method from the event</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">
    ///   Object method that will be unsubscribes from the event
    /// </typeparam>
    /// <param name="instance">Instance on which the object method was subscribed</param>
    /// <returns>True if the object method was subscribed and has been unsubscribed</returns>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...) const>
    bool Unsubscribe(const TClass *instance) {
      return Unsubscribe(DelegateType::template Create<TClass, TMethod>(instance));
    }

    /// <summary>Unsubscribes the specified delegate from the event</summary>
    /// <param name="delegate">Delegate that will be unsubscribed</param>
    /// <returns>True if the callback was found and unsubscribed, false otherwise</returns>
    public: bool Unsubscribe(const DelegateType &delegate);

    /// <summary>Switches the event from stack-stored subscribers to heap-stored</summary>
    /// <remarks>
    ///   For internal use only; this must only be called when the subscriber count is
    ///   equal to the built-in stack storage space and must be followed immediately by
    ///   adding another subscriber (because the subscriberCount is the decision variable
    ///   for the event to know whether to assume heap storage or stack storage).
    /// </remarks>
    private: void convertFromStackToHeapAllocation() {
      const static std::size_t initialCapacity = BuiltInSubscriberCount * 8;
      std::uint8_t *initialBuffer = new std::uint8_t[
        sizeof(DelegateType[2]) * initialCapacity / 2
      ]; // CHECK: Do we risk alignment issues here?

      std::copy_n(
        this->stackMemory,
        sizeof(DelegateType[2]) * BuiltInSubscriberCount / 2,
        initialBuffer
      );

      this->heapMemory.ReservedSubscriberCount = initialCapacity;
      this->heapMemory.Buffer = initialBuffer;
    }

    /// <summary>Increases the size of the heap-allocated list of event subscribers</summary>
    private: void growHeapAllocatedList() {
      std::size_t newCapacity = this->heapMemory.ReservedSubscriberCount * 2;
      std::uint8_t *newBuffer = new std::uint8_t[
        sizeof(DelegateType[2]) * newCapacity / 2
      ]; // CHECK: Do we risk alignment issues here?

      std::copy_n(
        this->heapMemory.Buffer,
        sizeof(DelegateType[2]) * this->subscriberCount / 2,
        newBuffer
      );

      std::swap(this->heapMemory.Buffer, newBuffer);
      this->heapMemory.ReservedSubscriberCount = newCapacity;
      delete []newBuffer;
    }

    /// <summary>Moves the event's subscriber list back into its own stack storage</summary>
    /// <remarks>
    ///   This must only be called if the event has just shrunk to the number of subscribers
    ///   that can fit into built-in stack space for storing event subscriptions. The call is
    ///   mandatory (because the subscriberCount is the decision variable for the event to
    ///   know whether to assume heap storage or stack storage).
    /// </remarks>
    private: void convertFromHeapToStackAllocation() {
      std::uint8_t *oldBuffer = this->heapMemory.Buffer;

      std::copy_n(
        oldBuffer,
        sizeof(DelegateType[2]) * BuiltInSubscriberCount / 2,
        this->stackMemory
      );

      delete []oldBuffer;
    }

    /// <summary>Information about subscribers if the list is moved to the heap</summary>
    private: struct HeapAllocatedSubscribers {

      /// <summary>Number of subscribers for which space has been reserved on the heap</summary>
      public: std::size_t ReservedSubscriberCount;
      /// <summary>Dynamically allocated memory the subscribers are stored in</summary>
      public: alignas(DelegateType) std::uint8_t *Buffer;

    };

    /// <summary>Number of subscribers that have registered to the event</summary>
    private: std::size_t subscriberCount;
    /// <summary>Stores the first n subscribers inside the event's own memory</summary>
    private: union {
      HeapAllocatedSubscribers heapMemory;
      std::uint8_t stackMemory[sizeof(DelegateType[BuiltInSubscriberCount])];
    };

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  template<typename TOutputIterator>
  void Event<TResult(TArguments...)>::EmitAndCollect(
    TOutputIterator results, TArguments&&... arguments
  ) const {
    std::size_t knownSubscriberCount = this->subscriberCount;

    const DelegateType *subscribers;
    std::size_t index = 0;

    // Is the subscriber list currently on the stack?
    if(knownSubscriberCount <= BuiltInSubscriberCount) {
ProcessStackSubscribers:
      subscribers = reinterpret_cast<const DelegateType *>(this->stackMemory);
      while(index < knownSubscriberCount) {
        *results = subscribers[index](std::forward<TArguments>(arguments)...);
        ++results;
        if(this->subscriberCount == knownSubscriberCount) {
          ++index; // Only increment if the current callback wasn't unsubscribed
        } else if(this->subscriberCount > knownSubscriberCount) {
          ++index;
          if(knownSubscriberCount > BuiltInSubscriberCount) {
            knownSubscriberCount = this->subscriberCount;
            goto ProcessHeapSubscribers;
          }
          knownSubscriberCount = this->subscriberCount;
        } else {
          knownSubscriberCount = this->subscriberCount;
        }
      }

      return;
    }

    // The subscriber list is currently on the heap
    {
ProcessHeapSubscribers:
      subscribers = reinterpret_cast<const DelegateType *>(this->heapMemory.Buffer);
      while(index < knownSubscriberCount) {
        *results = subscribers[index](std::forward<TArguments>(arguments)...);
        ++results;
        if(this->subscriberCount == knownSubscriberCount) {
          ++index; // Only increment if the current callback wasn't unsubscribed
        } else if(this->subscriberCount < knownSubscriberCount) {
          if(knownSubscriberCount <= BuiltInSubscriberCount) {
            knownSubscriberCount = this->subscriberCount;
            goto ProcessStackSubscribers;
          }
          knownSubscriberCount = this->subscriberCount;
        } else {
          ++index;
          knownSubscriberCount = this->subscriberCount;
          // In case more heap memory had to be allocated
          subscribers = reinterpret_cast<const DelegateType *>(this->heapMemory.Buffer);
        }
      }

      return;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  void Event<TResult(TArguments...)>::Emit(TArguments... arguments) const {
    std::size_t knownSubscriberCount = this->subscriberCount;

    const DelegateType *subscribers;
    std::size_t index = 0;

    // Is the subscriber list currently on the stack?
    if(knownSubscriberCount <= BuiltInSubscriberCount) {
ProcessStackSubscribers:
      subscribers = reinterpret_cast<const DelegateType *>(this->stackMemory);
      while(index < knownSubscriberCount) {
        subscribers[index](std::forward<TArguments>(arguments)...);
        if(this->subscriberCount == knownSubscriberCount) {
          ++index; // Only increment if the current callback wasn't unsubscribed
        } else if(this->subscriberCount > knownSubscriberCount) {
          ++index;
          if(knownSubscriberCount > BuiltInSubscriberCount) {
            knownSubscriberCount = this->subscriberCount;
            goto ProcessHeapSubscribers;
          }
          knownSubscriberCount = this->subscriberCount;
        } else {
          knownSubscriberCount = this->subscriberCount;
        }
      }

      return;
    }

    // The subscriber list is currently on the heap
    {
ProcessHeapSubscribers:
      subscribers = reinterpret_cast<const DelegateType *>(this->heapMemory.Buffer);
      while(index < knownSubscriberCount) {
        subscribers[index](std::forward<TArguments>(arguments)...);
        if(this->subscriberCount == knownSubscriberCount) {
          ++index; // Only increment if the current callback wasn't unsubscribed
        } else if(this->subscriberCount < knownSubscriberCount) {
          if(knownSubscriberCount <= BuiltInSubscriberCount) {
            knownSubscriberCount = this->subscriberCount;
            goto ProcessStackSubscribers;
          }
          knownSubscriberCount = this->subscriberCount;
        } else {
          ++index;
          knownSubscriberCount = this->subscriberCount;
          // In case more heap memory had to be allocated
          subscribers = reinterpret_cast<const DelegateType *>(this->heapMemory.Buffer);
        }
      }

      return;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TResult, typename... TArguments>
  bool Event<TResult(TArguments...)>::Unsubscribe(const DelegateType &delegate) {
    if(this->subscriberCount <= BuiltInSubscriberCount) {
      DelegateType *subscribers = reinterpret_cast<DelegateType *>(this->stackMemory);
      for(std::size_t index = 0; index < this->subscriberCount; ++index) {
        if(subscribers[index] == delegate) {
          std::size_t lastSubscriberIndex = this->subscriberCount - 1;
          subscribers[index] = subscribers[lastSubscriberIndex];
          --this->subscriberCount;
          return true;
        }
      }
    } else {
      DelegateType *subscribers = reinterpret_cast<DelegateType *>(this->heapMemory.Buffer);
      std::size_t lastSubscriberIndex = this->subscriberCount;
      if(lastSubscriberIndex > 0) {
        --lastSubscriberIndex;

        // Tiny optimization. Often the removed event is the last one registered
        if(subscribers[lastSubscriberIndex] == delegate) {
          --this->subscriberCount;
          if(this->subscriberCount <= BuiltInSubscriberCount) {
            convertFromHeapToStackAllocation();
          }
          return true;
        }
        for(std::size_t index = 0; index < lastSubscriberIndex; ++index) {
          if(subscribers[index] == delegate) {
            subscribers[index] = subscribers[lastSubscriberIndex];
            --this->subscriberCount;
            if(this->subscriberCount <= BuiltInSubscriberCount) {
              convertFromHeapToStackAllocation();
            }
            return true;
          }
        }
      }
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_EVENT_H
