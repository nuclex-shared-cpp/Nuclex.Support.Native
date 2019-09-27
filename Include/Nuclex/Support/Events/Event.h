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

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //


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