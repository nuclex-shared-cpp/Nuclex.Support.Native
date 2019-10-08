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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_GEOMETRY_SOURCE 1

#include "Nuclex/Support/Events/Event.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Free function used to test event subscriptions</summary>
  void freeFunction(int) { }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy class used to test event subscriptions</summary>
  class MockedSubscriber {

    /// <summary>Initializes a new mocked subscriber</summary>
    public: MockedSubscriber() :
      ReceivedNotificationCount(0),
      LastSomethingParameterValue(0) {}

    /// <summary>Method that can be subscribed to an event for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void Notify(int something) {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
    }

    /// <summary>Number of calls to Notify() the instance has observed</summary>
    public: std::size_t ReceivedNotificationCount;
    /// <summary>Value that was last passed to the Notify() method</summary>
    public: int LastSomethingParameterValue;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EventsCanBeCreated) {
    EXPECT_NO_THROW(
      Event<void(int something)> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, FreeFunctionsCanBeSubscribed) {
    Event<void(int something)> test;
    Delegate<void(int something)> subscriber = (
      Delegate<void(int something)>::Create<freeFunction>()
    );
    test.Subscribe(subscriber);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EventCanHandleManySubscriptions) {
    Event<void(int something)> test;
    Delegate<void(int something)> subscriber = (
      Delegate<void(int something)>::Create<freeFunction>()
    );
    for(std::size_t index = 0; index < 20; ++index) {
      test.Subscribe(subscriber);
    }
  }

#if 0
  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, FreeFunctionsCanBeUnsubscribed) {
    Event<void(int something)> test;
    test.Subscribe(freeFunction);
    test.Unsubscribe(freeFunction);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, MethodsCanBeSubscribed) {
    using std::placeholders::_1;

    Event<void(int something)> test;
    MockedSubscriber mock;
    test.Subscribe(std::bind(&MockedSubscriber::Notify, mock, _1));
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
