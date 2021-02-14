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

#include "Nuclex/Support/Collections/ConcurrentRingBuffer.h"
#include "BufferTest.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A concurrent single producer, single consumer ring buffer of integers</summary>
  typedef Nuclex::Support::Collections::ConcurrentRingBuffer<
    int, Nuclex::Support::Collections::ConcurrentAccessBehavior::MultipleProducersSingleConsumer
  > IntegerRingBuffer;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      IntegerRingBuffer test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, CanReportCapacity) {
    IntegerRingBuffer test(124);
    EXPECT_EQ(test.GetCapacity(), 124U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleItemsCanBeAppended) {
    IntegerRingBuffer test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleAppendFailsIfBufferFull) {
    IntegerRingBuffer test(3);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_FALSE(test.TryAppend(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, ItemsCanBeCounted) {
    IntegerRingBuffer test(3);
    EXPECT_EQ(test.Count(), 0U);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_EQ(test.Count(), 1U);
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_EQ(test.Count(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, BufferCanBeEmpty) {
    IntegerRingBuffer test(5);
    
    int value;
    EXPECT_FALSE(test.TryTake(value)); // Starts out empty
    EXPECT_TRUE(test.TryAppend(100));
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_FALSE(test.TryTake(value)); // Was emptied again with call above
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleItemsCanBeRead) {
    IntegerRingBuffer test(5);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));

    int value;
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 123);
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 456);
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 789);
    EXPECT_FALSE(test.TryTake(value));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
