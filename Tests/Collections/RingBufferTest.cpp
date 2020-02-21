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

#include "Nuclex/Support/Collections/RingBuffer.h"
#include <gtest/gtest.h>

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  class TestItem {

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {
#if 0
  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      RingBuffer<std::uint8_t> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, NewInstanceContainsNoItems) {
    RingBuffer<std::uint8_t> test;
    EXPECT_EQ(test.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, StartsWithNonZeroDefaultCapacity) {
    RingBuffer<std::uint8_t> test;
    EXPECT_GT(test.GetCapacity(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, CanStartWithCustomCapacity) {
    RingBuffer<std::uint8_t> test(512U);
    EXPECT_GE(test.GetCapacity(), 512U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, HasCopyConstructor) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Append(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    RingBuffer<std::uint8_t> copy(test);

    EXPECT_EQ(copy.Count(), 10U);

    std::uint8_t retrieved[10];
    copy.Dequeue(retrieved, 10);

    EXPECT_EQ(copy.Count(), 0U);
    EXPECT_EQ(test.Count(), 10U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, HasMoveConstructor) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Append(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    RingBuffer<std::uint8_t> moved(std::move(test));

    EXPECT_EQ(moved.Count(), 10U);

    std::uint8_t retrieved[10];
    moved.Dequeue(retrieved, 10);

    EXPECT_EQ(moved.Count(), 0U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, ItemsCanBeAppended) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    test.Append(items, 128);
    
    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, DequeuingFromEmptyBufferCausesException) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    EXPECT_THROW(
      test.Dequeue(items, 1),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, DequeuingTooManyItemsCausesException) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[100];
    test.Append(items, 99);

    EXPECT_THROW(
      test.Dequeue(items, 100),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, DequeuingTooManyItemsInWrappedBufferCausesException) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    std::vector<std::uint8_t> retrieved(capacity);

    std::size_t oneThirdCapacity = capacity / 3;
    test.Append(&items[0], oneThirdCapacity * 2);
    test.Dequeue(&retrieved[0], oneThirdCapacity);
    test.Append(&items[0], oneThirdCapacity * 2);
    test.Dequeue(&retrieved[0], oneThirdCapacity);

    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    EXPECT_THROW(
      test.Dequeue(&items[0], oneThirdCapacity * 2 + 1),
      std::logic_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, ItemsCanBeAppendedAndDequeued) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    for(std::size_t index = 0; index < 128; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }
    test.Append(items, 128);
    
    EXPECT_EQ(test.Count(), 128U);

    std::uint8_t retrieved[128];
    test.Dequeue(retrieved, 128);

    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < 128; ++index) {
      EXPECT_EQ(retrieved[index], static_cast<std::uint8_t>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, AppendAndDequeueHandleWrapAround) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();
    
    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Append(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle of the ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Dequeue(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add another 2/3rds to the ring buffer. The write must wrap around.
    test.Append(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3);

    // Finally, retrieve everything. The read must wrap around.
    test.Dequeue(&retrieved[0], oneThirdCapacity * 3);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < oneThirdCapacity * 2; ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, WholeBufferCanBeFilledAndEmptied) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to its current capacity
    test.Append(&items[0], capacity);
    EXPECT_EQ(test.Count(), capacity);

    // Remove the first 1/3rd, we end up with data in the middle ofthe ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Dequeue(&retrieved[0], capacity);
    EXPECT_EQ(test.Count(), 0);

    for(std::size_t index = 0; index < capacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, AppendCanHitBufferEnd) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();
    
    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Append(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle ofthe ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Dequeue(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add exactly the amount of items it takes to hit the end of the buffer
    std::size_t remainingItemCount = capacity - (oneThirdCapacity * 2);
    test.Append(&items[0], remainingItemCount);
    EXPECT_EQ(test.Count(), oneThirdCapacity + remainingItemCount);

    // If there's a karfluffle or off-by-one problem when hitting the end index,
    // this next call might blow up
    test.Append(&items[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), capacity);

    // Read all of the data from the ring buffer so we can check it
    test.Dequeue(&retrieved[0], capacity);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < (capacity - oneThirdCapacity * 2); ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }
    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index + (capacity - oneThirdCapacity)], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferTest, DequeueCanHitBufferEnd) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();
    
    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Append(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle of the ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Dequeue(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add another 2/3rds to the ring buffer. The write must wrap around.
    test.Append(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3);

    // Finally, retrieve just enough bytes to hit the end.
    test.Dequeue(&retrieved[0], capacity - oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3 - (capacity - oneThirdCapacity));

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < capacity - oneThirdCapacity * 2; ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }

    // If there's a karfluffle or off-by-one problem when moving the start index,
    // this next call might blow up
    std::size_t remainingByteCount = oneThirdCapacity * 3 - (capacity - oneThirdCapacity);
    test.Dequeue(&retrieved[0], remainingByteCount);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < remainingByteCount; ++index) {
      EXPECT_EQ(
        retrieved[index],
        items[index + capacity - oneThirdCapacity * 2]
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //
#endif
}}} // namespace Nuclex::Support::Collections