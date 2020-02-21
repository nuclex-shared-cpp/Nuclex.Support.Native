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

#include "Nuclex/Support/Collections/ShiftBuffer.h"
#include <gtest/gtest.h>

#include <vector> // for std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Unique number generator for the test item class</summary>
  int NextUniqueNumber = 0;

  // ------------------------------------------------------------------------------------------- //

  struct TestItemStats {

    /// <summary>Number of times the item was the source of a copy</summary>
    public: int CopyCount = 0;
    /// <summary>Number of times the item was the source of a move</summary>
    public: int MoveCount = 0;
    /// <summary>Numberof times an associated item was destroyed</summary>
    public: int DestroyCount = 0;
    /// <summary>Whether the associated item's copy constructor should fail</summary>
    public: bool ThrowOnCopy = false;
    /// <summary>Whether the associated item's move constructor should fail</summary>
    public: bool ThrowOnMove = false;
    /// <summary>Unique number by which this instance can be identified</summary>
    public: int UniqueNumber = NextUniqueNumber++;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Dummy item to test correct copy and move semantics of the shift buffer</summary>
  class TestItem {

    /// <summary>Initializes a new item</summary>
    public: TestItem(const std::shared_ptr<TestItemStats> &stats) :
      stats(stats) {}

    /// <summary>Initializes an item as copy of another item</summary>
    /// <param name="other">Other item that will be copied</param>
    public: TestItem(const TestItem &other) :
      stats(other.stats) {
      ++this->stats->CopyCount;
    }

    /// <summary>Initializes an item by taking over an existing item</summary>
    /// <param name="other">Other item that will be taken over</param>
    public: TestItem(TestItem &&other) :
      stats(std::move(other.stats)) {
      ++this->stats->MoveCount;
    }

    /// <summary>Destroys the item</summary>
    public: ~TestItem() {
      ++this->stats->DestroyCount;
    }

    /// <summary>
    ///   Status tracker for the instance, used by tests to verify expected actions took place
    /// </summary>
    private: std::shared_ptr<TestItemStats> stats;

  };

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::shared_ptr<TestItemStats>> makeStats(std::size_t count) {
    std::vector<std::shared_ptr<TestItemStats>> stats;
    stats.reserve(count);

    for(std::size_t index = 0; index < count; ++index) {
      stats.emplace_back(std::make_shared<TestItemStats>());
    }

    return stats;
  }

  // ------------------------------------------------------------------------------------------- //

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

  void resetCounter(
    std::vector<std::shared_ptr<TestItemStats>> &stats
  ) {
    std::size_t statCount = stats.size();

    for(std::size_t index = 0; index < statCount; ++index) {
      stats[index]->CopyCount = 0;
      stats[index]->MoveCount = 0;
      stats[index]->DestroyCount = 0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      ShiftBuffer<std::uint8_t> trivialTest;
      ShiftBuffer<TestItem> customTest;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, NewInstanceContainsNoItems) {
    ShiftBuffer<std::uint8_t> trivialTest;
    EXPECT_EQ(trivialTest.Count(), 0U);

    ShiftBuffer<TestItem> customTest;
    EXPECT_EQ(customTest.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, StartsWithNonZeroDefaultCapacity) {
    ShiftBuffer<std::uint8_t> trivialTest;
    EXPECT_GT(trivialTest.GetCapacity(), 0U);

    ShiftBuffer<TestItem> customTest;
    EXPECT_GT(customTest.GetCapacity(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, CanStartWithCustomCapacity) {
    ShiftBuffer<std::uint8_t> trivialTest(512U);
    EXPECT_GE(trivialTest.GetCapacity(), 512U);

    ShiftBuffer<TestItem> customTest(512U);
    EXPECT_GE(customTest.GetCapacity(), 512U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, HasCopyConstructor) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftBuffer<std::uint8_t> copy(test);

    EXPECT_EQ(copy.Count(), 10U);

    std::uint8_t retrieved[10];
    copy.Read(retrieved, 10);

    EXPECT_EQ(copy.Count(), 0U);
    EXPECT_EQ(test.Count(), 10U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, HasMoveConstructor) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftBuffer<std::uint8_t> moved(std::move(test));

    EXPECT_EQ(moved.Count(), 10U);

    std::uint8_t retrieved[10];
    moved.Read(retrieved, 10);

    EXPECT_EQ(moved.Count(), 0U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeAppended) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeAppendedWithMoveSemantics) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    test.Shove(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeReadAndWritten) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    for(std::size_t index = 0; index < 128; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);

    std::uint8_t retrieved[128];
    test.Read(retrieved, 128);

    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < 128; ++index) {
      EXPECT_EQ(retrieved[index], static_cast<std::uint8_t>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, MoveSemanticsAreUsedWhenCapacityChanges) {
    std::vector<std::shared_ptr<TestItemStats>> stats = makeStats(17);
    std::vector<TestItem> items;
    makeItems(items, stats);

    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 0);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
    }

    ShiftBuffer<TestItem> test(16);
    test.Write(&items[0], 16);
    
    for(std::size_t index = 0; index < 16; ++index) {
      EXPECT_EQ(stats[index]->CopyCount, 1);
      EXPECT_EQ(stats[index]->MoveCount, 0);
      EXPECT_EQ(stats[index]->DestroyCount, 0);
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
