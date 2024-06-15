#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

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
      test.Get(25),
      Errors::KeyNotFoundError
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, RetrievalCanIgnoreMissingItems) {
    SequentialSlotCache<std::size_t, int> test(32);

    int obtainedValue = 0;
    bool wasObtained = test.TryGet(12, obtainedValue);
    EXPECT_FALSE(wasObtained);
    EXPECT_EQ(obtainedValue, 0);

    bool wasInserted = test.TryInsert(12, 20384);
    EXPECT_TRUE(wasInserted);
    EXPECT_EQ(test.Count(), 1U);

    wasObtained = test.TryGet(12, obtainedValue);
    EXPECT_TRUE(wasObtained);
    EXPECT_EQ(obtainedValue, 20384);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, ItemsCanBeTaken) {
    SequentialSlotCache<std::size_t, int> test(32);

    bool wasInserted = test.TryInsert(30, 53345);
    EXPECT_TRUE(wasInserted);
    EXPECT_EQ(test.Count(), 1U);

    int takenValue = 0;
    bool wasTaken = test.TryTake(30, takenValue);
    EXPECT_TRUE(wasTaken);
    EXPECT_EQ(takenValue, 53345);
    EXPECT_EQ(test.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, TakingWontThrowOnMissingKeys) {
    SequentialSlotCache<std::size_t, int> test(32);

    int takenValue = 0;
    bool wasTaken = test.TryTake(23, takenValue);
    EXPECT_FALSE(wasTaken);
    EXPECT_EQ(takenValue, 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SequentialSlotCacheTest, EvictKeepsRecentlyAccessedItems) {
    SequentialSlotCache<std::size_t, int> test(32);

    test.Insert(2, 202);
    test.Insert(4, 404);
    test.Insert(6, 606);
    test.Insert(8, 808);
    test.Insert(10, 999);

    test.Get(4); // Move item 4 back to top of most recently accessed

    EXPECT_EQ(test.Count(), 5U);
    test.EvictDownTo(3);
    EXPECT_EQ(test.Count(), 3U);

    int obtainedValue;
    EXPECT_FALSE(test.TryGet(2, obtainedValue));
    EXPECT_TRUE(test.TryGet(4, obtainedValue));
    EXPECT_FALSE(test.TryGet(6, obtainedValue));
    EXPECT_TRUE(test.TryGet(8, obtainedValue));
    EXPECT_TRUE(test.TryGet(10, obtainedValue));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
