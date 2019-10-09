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
  class Mock {

    /// <summary>Initializes a new mocked subscriber</summary>
    public: Mock() :
      ReceivedNotificationCount(0),
      LastSomethingParameterValue(0) {}

    /// <summary>Method that can be subscribed to an event for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void Notify(int something) {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
    }

    /// <summary>Method that can be subscribed to an event for testing</summary>
    /// <param name="something">Dummy integer value that will be remembered</param>
    public: void ConstNotify(int something) const {
      this->LastSomethingParameterValue = something;
      ++this->ReceivedNotificationCount;
    }

    /// <summary>Number of calls to Notify() the instance has observed</summary>
    public: mutable std::size_t ReceivedNotificationCount;
    /// <summary>Value that was last passed to the Notify() method</summary>
    public: mutable int LastSomethingParameterValue;

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
    test.Subscribe<freeFunction>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EventCanHandleManySubscriptions) {
    Event<void(int something)> test;
    for(std::size_t index = 0; index < 32; ++index) {
      test.Subscribe<freeFunction>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, FreeFunctionsCanBeUnsubscribed) {
    Event<void(int something)> test;
    test.Subscribe<freeFunction>();
    
    bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, EachSubscriptionRequiresOneUnsubscription) {
    Event<void(int something)> test;

    for(std::size_t index = 0; index < 32; ++index) {
      test.Subscribe<freeFunction>();
    }

    for(std::size_t index = 0; index < 32; ++index) {
      bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
      EXPECT_TRUE(wasUnsubscribed);
    }

    bool wasUnsubscribed = test.Unsubscribe<freeFunction>();
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ObjectMethodsCanBeSubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ObjectMethodsCanBeUnsubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeSubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeUnsubscribed) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }


  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeSubscribedOnConstInstance) {
    Event<void(int something)> test;

    const Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, ConstObjectMethodsCanBeUnsubscribedOnConstInstance) {
    Event<void(int something)> test;

    const Mock mock;
    test.Subscribe<Mock, &Mock::ConstNotify>(&mock);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);
    wasUnsubscribed = test.Unsubscribe<Mock, &Mock::ConstNotify>(&mock);
    EXPECT_FALSE(wasUnsubscribed);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EventTest, NotificatinsAreSentToSubscribers) {
    Event<void(int something)> test;

    Mock mock;
    test.Subscribe<Mock, &Mock::Notify>(&mock);

    EXPECT_EQ(mock.ReceivedNotificationCount, 0);
    EXPECT_EQ(mock.LastSomethingParameterValue, 0);

    test(135);

    EXPECT_EQ(mock.ReceivedNotificationCount, 1);
    EXPECT_EQ(mock.LastSomethingParameterValue, 135);

    bool wasUnsubscribed = test.Unsubscribe<Mock, &Mock::Notify>(&mock);
    EXPECT_TRUE(wasUnsubscribed);

    test(135);

    EXPECT_EQ(mock.ReceivedNotificationCount, 1);
    EXPECT_EQ(mock.LastSomethingParameterValue, 135);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
