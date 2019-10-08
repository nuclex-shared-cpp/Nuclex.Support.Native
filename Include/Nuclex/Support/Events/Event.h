#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_EVENTS_EVENT_H
#define NUCLEX_SUPPORT_EVENTS_EVENT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Delegate.h"

#include <functional>
#include <algorithm>
#include <vector>

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  // Prototype, required for variable argument template
  template<typename> class Event;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a list of subscribers that receive callback when the event fires</summary>
  /// <typeparam name="TResult">Type that will be returned from the method</typeparam>
  /// <typeparam name="TArguments">Types of the arguments accepted by the callback</typeparam>
  /// <remarks>
  ///   <para>
  ///     An event should be equivalent in size to 5 pointers (depending on the
  ///     value of the <see cref="BuiltInSubscriberCount" /> constant))
  ///   </para>
  /// </remarks>
  template<typename TResult, typename... TArguments>
  class Event<TResult(TArguments...)> {

    /// <summary>Number of subscribers the event can subscribe withou allocating memory</summary>
    /// <remarks>
    ///   To reduce complexity, this value is unchangeable. It is the number off subscriber
    ///   slots that are baked into the event, enabling it to handle a small number of
    ///   subscribers without allocating heap memory. Each slot takes the size of a delegate,
    ///   64 bits on a 32 bit system or 128 bits on a 64 bit system.
    /// </remarks>
    private: const static std::size_t BuiltInSubscriberCount = 2;

    /// <summary>Type of value that will be returned by the delegate</summary>
    public: typedef TResult ResultType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    public: typedef TResult CallType(TArguments...);

    /// <summary>Type the will be returned by the event itself</summary>
    public: typedef typename std::conditional<
      std::is_void<TResult>::value, std::vector<TResult>, void
    >::type EventResultType;

    /// <summary>Initializes a new event</summary>
    public: Event() :
      subscriberCount(0) {}

    public: ~Event() {
      // TODO: Check heap/stack storage
      // TODO: Call Delegate destructors
    }

    /// <summary>Fires the event, calling all subscribers and collecting the results</summary>
    /// <param name="arguments">Arguments that will be passed to the event</param>
    /// <returns>
    ///   Nothing if the event's return type is void, a collection of all events results otherwise
    /// </returns>
    public: EventResultType operator()(TArguments... arguments) {
      throw -1;
    }

    //public: template<typename = typename enable_if<std::is_void<TResult>::value>::value, void>::type>
    //void operator(EventResultType &results)() {}

#if 0
    /// <summary>Resets the delegate to the specified free function</summary>
    /// <typeparam name="TMethod">Free function that will be called by the delegate</typeparam>
    public: template<TResult(*TMethod)(TArguments...)>
    void Subscribe() {
      auto x = Delegate<void(int something)>::Create<&Event::shit>();
      //auto x = Delegate<TResult(TArguments...)>::Create<TMethod>();
      //Subscribe(Delegate<TResult(TArguments...)>::Create<TMethod>());
    }

    /// <summary>Resets the delegate to the specified object method</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Free function that will be called by the delegate</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    void Subscribe(TClass *instance) {
      //Delegate<TResult(TArguments...)>::Create<TClass, TMethod>(instance);
      //Subscribe(Delegate<TResult(TArguments...)>::Create<TClass, TMethod>(instance));
    }

    /// <summary>Resets the delegate to the specified const object method</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Free function that will be called by the delegate</typeparam>
    /// <param name="instance">Instance on which the object method will be called</param>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...) const>
    void Subscribe(const TClass *instance) {
      //Subscribe(Delegate<TResult(TArguments...)>::Create<TClass, TMethod>(instance));
    }
#endif

    /// <summary>Subscribes the specified delegate to the event</summary>
    /// <param name="delegate">Delegate that will be subscribed</param>
    public: void Subscribe(const Delegate<TResult(TArguments...)> &delegate) {
      if(this->subscriberCount < BuiltInSubscriberCount) {
        reinterpret_cast<Delegate<TResult(TArguments...)> *>(this->stackMemory)[
          this->subscriberCount
        ] = delegate;
      } else {
        if(this->subscriberCount == BuiltInSubscriberCount) {
          convertFromStackToHeapAllocation();
        } else if(this->subscriberCount >= this->heapMemory.ReservedSubscriberCount) {
          growHeapAllocatedList();
        }

        *reinterpret_cast<Delegate<TResult(TArguments...)> *>(this->heapMemory.Buffer) = (
          delegate
        );
        //this->heapMemory.Subscribers[this->subscriberCount] = delegate;
      }
      ++this->subscriberCount;
    }

    /// <summary>Switches the event from stack-stored subscribers to heap-stored</summary>
    /// <remarks>
    ///   For internal use only; this must only be called when the subscriber count is
    ///   equal to the built-in stack storage space and must be followed immediately by
    ///   adding another subscriber (because the subscriberCount is the decision variable
    ///   for the event to know whether to assume heap storage or stack storage).
    /// </remarks>
    private: void convertFromStackToHeapAllocation() {
      std::size_t initialCapacity = this->subscriberCount * 4;

      std::uint8_t *initialBuffer = new std::uint8_t[
        sizeof(Delegate<TResult(TArguments...)>[2]) * (initialCapacity / 2)
      ];
      std::copy_n(
        reinterpret_cast<Delegate<TResult(TArguments...)> *>(this->stackMemory),
        this->subscriberCount,
        reinterpret_cast<Delegate<TResult(TArguments...)> *>(initialBuffer)
      );

      this->heapMemory.ReservedSubscriberCount = initialCapacity;
      this->heapMemory.Buffer = initialBuffer;
    }

    /// <summary>Increases the size of the heap-allocated list of event subscribers</summary>
    private: void growHeapAllocatedList() {
      std::size_t newCapacity = this->heapMemory.ReservedSubscriberCount * 2;

      std::uint8_t *newBuffer = new std::uint8_t[
        sizeof(Delegate<TResult(TArguments...)>[2]) * (newCapacity / 2)
      ];
      std::copy_n(
        reinterpret_cast<Delegate<TResult(TArguments...)> *>(this->heapMemory.Buffer),
        this->subscriberCount,
        reinterpret_cast<Delegate<TResult(TArguments...)> *>(newBuffer)
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
    private: void convertFromHeapToStackAllocated() {
      throw "TODO";
    }

    /// <summary>Information about subscribers if the list is moved to the heap</summary>
    private: struct HeapAllocatedSubscribers {

      /// <summary>Number of subscribers for which space has been reserved on the heap</summary>      
      public: std::size_t ReservedSubscriberCount;
      /// <summary>Dynamically allocated memory  the subscribers are stored in</summary>
      public: std::uint8_t *Buffer;

    };

    /// <summary>Number of subscribers that have registered to the event</summary>
    private: std::size_t subscriberCount;
    /// <summary>Stores the first n subscribers inside the event's own memory</summary>
    private: union {
      HeapAllocatedSubscribers heapMemory;
      std::uint8_t stackMemory[
        sizeof(Delegate<TResult(TArguments...)>[BuiltInSubscriberCount])
      ];
    };

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_EVENT_H

#if 0

class EventSubscriber {
  
  private: class Subscription {
    public: virtual ~Subscription() {}
    public: virtual void Disconnect() = 0;
    //public: 
  };


};

class Event {
  
};

void mooh(int x) {}
class cow {
  public: void mooh(int x) {}
};

/*
class EventSubscription {
  public: ~EventSubscription() {}
  public: void 
};
*/

template<typename TArgument1>
class Event1 {

  #pragma region class Subscription

  /// <summary>Stores a function or method pointer for an event subscription</summary>
  private: class Subscription {

    /// <summary>Initializes a new event subscription</summary>
    /// <param name="event">Event that owns the subscription</summary>
    /// <param name="functionType">Type of the function the subscription calls</param>
    public: Subscription(Event1 *event, const std::type_info &functionType) :
      Event(event),
      FunctionType(functionType) {}

    /// <summary>Frees all memory used by the event subscription</summary>
    public: virtual ~Subscription() {}

    /// <summary>Calls the subscribed method</summary>
    /// <param name="argument1">First argument that will be passed to the subscriber</param>
    public: virtual void Call(TArgument1 argument1) = 0;

    /// <summary>Type of function or member function that is subscribed</summary>
    public: const std::type_info &FunctionType;
    /// <summary>Event this subscription belongs to</summary>
    protected: Event1 *Event;

  };

  #pragma endregion // class Subscription

  #pragma region class MemberFunctionSubscription
  
  /// <summary>Stores an object method pointer for an event subscription</summary>
  private: template<typename TSubscriber>
  class MemberFunctionSubscription : public Subscription {

    /// <summary>Initializes a new object method event subscription</summary>
    /// <param name="event">Event that owns the subscription</summary>
    /// <param name="subscriber">Subscribing object the method will be called on</param>
    /// <param name="method">Method that will be called when the event fires</param>
    public: MemberFunctionSubscription(
      Event1 *event, TSubscriber *subscriber, void (TSubscriber::*method)(TArgument1)
    ) :
      Subscription(event, typeid(method)),
      Subscriber(subscriber),
      Method(method),
      subscriberForDisconnect(nullptr) {
      registerAutomaticDisconnect(subscriber);
    }

    /// <summary>Frees all memory used by the event subscription</summary>
    public: virtual ~MemberFunctionSubscription() {}

    /// <summary>Calls the subscribed object method</summary>
    /// <param name="argument1">First argument that will be passed to the subscriber</param>
    public: virtual void Call(TArgument1 argument1) {
      ((this->Subscriber)->*Method)(argument1);
    }

    
    public: virtual bool IsCalling(void *function) const { return false; }

    /// <summary>Dummy method for when automatic disconnects are not supported</summary>
    private: void registerAutomaticDisconnect(void *) {}

    /// <sumamry>Registers the event subscriber for automatic disconnection</summary>
    /// <param name="subscriberForDisconnect">Event subscriber to notify on disconnect</param>
    private: void registerAutomaticDisconnect(EventSubscriber *subscriberForDisconnect) {
      this->subscriberForDisconnect = subscriberForDisconnect;
    }

    /// <summary>Subscriber on which the object method will be invoked</summary>
    public: TSubscriber *Subscriber;
    /// <summary>Method that will be invoked on the subscriber</summary>
    public: void (TSubscriber::*Method)(TArgument1);
    /// <summary>Subscriber to use for automatic disconnects</summary>
    private: EventSubscriber *subscriberForDisconnect;

  };

  #pragma endregion // class MemberFunctionSubscription

  #pragma region class FunctionSubscription

  /// <summary>Stores an free function pointer for an event subscription</summary>
  public: class FunctionSubscription {

    /// <summary>Initializes a new free function event subscription</summary>
    /// <param name="event">Event that owns the subscription</summary>
    /// <param name="function">Function that will be called when the event fires</param>
    public: FunctionSubscription(Event1 *event, void (*function)(TArgument1)) :
      Subscription(Event1 *event, typeid(function)),
      function(function) {}

    /// <summary>Frees all memory used by the event subscription</summary>
    public: virtual ~FunctionSubscription() {}

    /// <summary>Calls the subscribed function</summary>
    /// <param name="argument1">First argument that will be passed to the subscriber</param>
    public: virtual void Call(TArgument1 argument1) {
      (this->*function)(argument1);
    }

    /// <summary>Function that will be invoked on the subscriber</summary>
    private: void (*function)(TArgument1);

  };

  #pragma endregion // class FunctionSubscription

  public: template<typename TSubscriber> void Connect(
    TSubscriber *subscriber, void (TSubscriber::*method)(TArgument1)
  ) {
    this->subscriptions.push_back(
      new MemberFunctionSubscription<TSubscriber>(this, subscriber, method)
    );
  }

  public: void Connect(void (*method)(TArgument1)) {
    
  }

  public: template<typename TSubscriber> void Disconnect(
    TSubscriber *subscriber,
    void (TSubscriber::*method)(TArgument1)
  ) {
    const std::type_info &methodType = typeid(method);

    for(std::size_t index = 0; index < this->subscriptions.size(); ++index) {
      
      if(this->subscriptions[index]->FunctionType == methodType) {
         MemberFunctionSubscription<TSubscriber> *subscription =
           static_cast<MemberFunctionSubscription<TSubscriber> *>(this->subscriptions[index]);

         bool isEqual =
           (subscription->Subscriber == subscriber) &&
           (subscription->Method == method);
         
      }
    }
    //void *methodAsVoid = method;
      //const std::type_info *type = typeof(method);

  }

  /// <summary>Retrieves the index of the specified member function subscription</summary>
  /// <param name="subscriber">Object that subscribed a member function</param>
  /// <param name="method">Object method whose subscription will be found</param>
  /// <returns>The index of the object method's subscription or -1 if not found</returns>
  private: template<typename TSubscriber> std::size_t find(
    TSubscriber *subscriber, void (TSubscriber::*method)(TArgument1)
  ) {
    const std::type_info &methodType = typeid(method);

    for(std::size_t index = 0; index < this->subscriptions.size(); ++index) {
      if(this->subscriptions[index]->FunctionType == methodType) {
        MemberFunctionSubscription<TSubscriber> *subscription =
          static_cast<MemberFunctionSubscription<TSubscriber> *>(this->subscriptions[index]);

        bool isEqual =
          (subscription->Subscriber == subscriber) &&
          (subscription->Method == method);

        if(isEqual) {
          return index;
        }
      }

      return static_cast<std::size_t>(-1);
    }
    //void *methodAsVoid = method;
      //const std::type_info *type = typeof(method);

  }

  /// <summary>Calls all event subscribers</summary>
  /// <param name="argument">First argument that will be passed to the subscribers</param>
  public: void Call(TArgument1 argument1) {
    for(std::size_t index = 0; index < this->subscriptions.size(); ++index) {
      this->subscriptions[index]->Call(argument1);
    }
  }

  /// <summary>Stores all subscribers of the event</summary>
  private: std::vector<Subscription *> subscriptions;

};

void test() {
  std::function<void(int)> fn(&mooh);
  std::function<void(cow *, int)> fn2(&cow::mooh);
  cow c;
  fn2(&c, 123);

  Event1<int> e;
  e.Connect(&c, &cow::mooh);
  e.Call(123);
  e.Disconnect(&c, &cow::mooh);
  
  
}

#endif