#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "Nuclex/Support/Collections/SequentialSlotCache.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, InstancesCanBeCreated) {
    typedef SequentialSlotCache<std::size_t, int> TestCache;
    EXPECT_NO_THROW(
      TestCache test(32);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, EmptyCacheCanBeCleared) {
    SequentialSlotCache<std::size_t, int> test(32);
    EXPECT_NO_THROW(
      test.Clear();
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, ItemsCanBeInserted) {
    SequentialSlotCache<std::size_t, int> test(32);
    EXPECT_EQ(test.Count(), 0U);

    bool wasFirstKeyUsage = test.Insert(15, 23897);
    EXPECT_TRUE(wasFirstKeyUsage);
    EXPECT_EQ(test.Count(), 1U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, ItemsCanBeRetrieved) {
    SequentialSlotCache<std::size_t, int> test(32);
    bool wasFirstKeyUsage = test.Insert(10, 12345);
    EXPECT_TRUE(wasFirstKeyUsage);

    int retrievedValue = test.Get(10);
    EXPECT_EQ(retrievedValue, 12345);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, InsertOverwritesValues) {
    SequentialSlotCache<std::size_t, int> test(32);

    bool wasFirstKeyUsage = test.Insert(20, 89732);
    EXPECT_TRUE(wasFirstKeyUsage);
    EXPECT_EQ(test.Count(), 1U);

    wasFirstKeyUsage = test.Insert(20, 54321);
    EXPECT_FALSE(wasFirstKeyUsage);
    EXPECT_EQ(test.Count(), 1U);

    int retrievedValue = test.Get(20);
    EXPECT_EQ(retrievedValue, 54321);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, ItemsCanBeInsertedWithoutOverwriting) {
    SequentialSlotCache<std::size_t, int> test(32);
    EXPECT_EQ(test.Count(), 0U);

    bool wasInserted = test.TryInsert(5, 45096);
    EXPECT_TRUE(wasInserted);
    EXPECT_EQ(test.Count(), 1U);

    wasInserted = test.TryInsert(5, 33412);
    EXPECT_FALSE(wasInserted);
    EXPECT_EQ(test.Count(), 1U);

    int retrievedValue = test.Get(5);
    EXPECT_EQ(retrievedValue, 45096);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, RetrievingMissingKeyThrowsException) {
    SequentialSlotCache<std::size_t, int> test(32);
    EXPECT_THROW(
      int value = test.Get(25),
      Errors::KeyNotFoundError
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
