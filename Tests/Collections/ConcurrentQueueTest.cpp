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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Collections/ConcurrentQueue.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(SingleProducerSingleConsumerConcurrentQueueTest, InstancesCanBeCreated) {
    typedef ConcurrentQueue<int, ConcurrentAccessBehavior::SingleProducerSingleConsumer> TestQueue;
    EXPECT_NO_THROW(
      TestQueue test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SingleProducerSingleConsumerConcurrentQueueTest, ItemsCanBeAppended) {
    typedef ConcurrentQueue<int, ConcurrentAccessBehavior::SingleProducerSingleConsumer> TestQueue;

    TestQueue test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SingleProducerSingleConsumerConcurrentQueueTest, QueueCanBeFull) {
    typedef ConcurrentQueue<int, ConcurrentAccessBehavior::SingleProducerSingleConsumer> TestQueue;

    TestQueue test(5);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_TRUE(test.TryAppend(321));
    EXPECT_TRUE(test.TryAppend(654));
    EXPECT_FALSE(test.TryAppend(987));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
