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

  /// <summary>Unique number generator for the test item class</summary>
  int NextUniqueNumber = 0;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tracks what happens with its associated test item</summary>
  struct TestItemStats {

    /// <summary>Number of times the item was the source of a copy</summary>
    public: int CopyCount = 0;
    /// <summary>Number of times the item was the source of a move</summary>
    public: int MoveCount = 0;
    /// <summary>Number of times an associated item was destroyed</summary>
    public: int DestroyCount = 0;
    /// <summary>Number of times an associated item was assigned to</summary>
    public: int OverwriteCount = 0;
    /// <summary>Whether the associated item's copy constructor should fail</summary>
    public: bool ThrowOnCopy = false;
    /// <summary>Whether the associated item's move constructor should fail</summary>
    public: bool ThrowOnMove = false;
    /// <summary>Unique number by which this instance can be identified</summary>
    public: int UniqueNumber = NextUniqueNumber++;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy item to test correct copy and move semantics of the ring buffer</summary>
  class TestItem {

    /// <summary>Initializes a new item</summary>
    public: TestItem(const std::shared_ptr<TestItemStats> &stats) :
      stats(stats) {}

    /// <summary>Initializes an item as copy of another item</summary>
    /// <param name="other">Other item that will be copied</param>
    public: TestItem(const TestItem &other) :
      stats(other.stats) {
      ++this->stats->CopyCount;
      if(this->stats->ThrowOnCopy) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
    }

    /// <summary>Initializes an item by taking over an existing item</summary>
    /// <param name="other">Other item that will be taken over</param>
    public: TestItem(TestItem &&other) :
      stats(other.stats) { // No move, we want to still track destruction
      ++this->stats->MoveCount;
      if(this->stats->ThrowOnMove) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
    }

    /// <summary>Destroys the item</summary>
    public: ~TestItem() {
      ++this->stats->DestroyCount;
    }

    /// <summary>Initializes an item as copy of another item</summary>
    /// <param name="other">Other item that will be copied</param>
    /// <returns>This instance</returns>
    public: TestItem &operator =(const TestItem &other) {
      ++this->stats->OverwriteCount;
      this->stats = other.stats;
      ++this->stats->CopyCount;
      if(this->stats->ThrowOnCopy) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
      return *this;
    }

    /// <summary>Initializes an item by taking over an existing item</summary>
    /// <param name="other">Other item that will be taken over</param>
    /// <returns>This instance</returns>
    public: TestItem &operator =(TestItem &&other) {
      ++this->stats->OverwriteCount;
      this->stats = other.stats; // No move, we want to still track destruction
      ++this->stats->MoveCount;
      if(this->stats->ThrowOnMove) {
        throw std::runtime_error(u8"Simulated error for unit testing");
      }
      return *this;
    }

    /// <summary>
    ///   Status tracker for the instance, used by tests to verify expected actions took place
    /// </summary>
    private: std::shared_ptr<TestItemStats> stats;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a vector of test item status structures</summary>
  /// <param name="count">Number of test item states that will be created</param>
  /// <returns>All test item states in a vector</returns>
  std::vector<std::shared_ptr<TestItemStats>> makeStats(std::size_t count) {
    std::vector<std::shared_ptr<TestItemStats>> stats;
    stats.reserve(count);

    for(std::size_t index = 0; index < count; ++index) {
      stats.emplace_back(std::make_shared<TestItemStats>());
    }

    return stats;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fills a vector with test items associated with test item states</summary>
  /// <param name="target">Vector that will receive the test items</param>
  /// <param name="stats">Test item states the created items will be associated with</param>
  void makeItems(
    std::vector<TestItem> &target, const std::vector<std::shared_ptr<TestItemStats>> &stats
  ) {
    std::size_t statCount = stats.size();

    target.reserve(statCount);
    for(std::size_t index = 0; index < statCount; ++index) {
      target.emplace_back(stats[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferDeathTest, DequeuingFromEmptyBufferTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    ASSERT_DEATH(
      test.Read(items, 1),
      u8""
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferDeathTest, DequeuingTooManyItemsTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::uint8_t items[100];
    test.Write(items, 99);

    ASSERT_DEATH(
      test.Read(items, 100),
      u8""
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingBufferDeathTest, DequeuingTooManyItemsInWrappedBufferTriggersAssertion) {
    RingBuffer<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    std::vector<std::uint8_t> retrieved(capacity);

    std::size_t oneThirdCapacity = capacity / 3;
    test.Write(&items[0], oneThirdCapacity * 2);
    test.Read(&retrieved[0], oneThirdCapacity);
    test.Write(&items[0], oneThirdCapacity * 2);
    test.Read(&retrieved[0], oneThirdCapacity);

    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    ASSERT_DEATH(
      test.Read(&items[0], oneThirdCapacity * 2 + 1),
      u8""
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
