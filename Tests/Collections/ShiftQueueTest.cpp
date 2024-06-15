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

#include "Nuclex/Support/Collections/ShiftQueue.h"
#include "BufferTest.h"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      ShiftQueue<std::uint8_t> trivialTest;
      ShiftQueue<TestItem> customTest;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, NewInstanceContainsNoItems) {
    ShiftQueue<std::uint8_t> trivialTest;
    EXPECT_EQ(trivialTest.Count(), 0U);

    ShiftQueue<TestItem> customTest;
    EXPECT_EQ(customTest.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, StartsWithNonZeroDefaultCapacity) {
    ShiftQueue<std::uint8_t> trivialTest;
    EXPECT_GT(trivialTest.GetCapacity(), 0U);

    ShiftQueue<TestItem> customTest;
    EXPECT_GT(customTest.GetCapacity(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, CanStartWithCustomCapacity) {
    ShiftQueue<std::uint8_t> trivialTest(512U);
    EXPECT_GE(trivialTest.GetCapacity(), 512U);

    ShiftQueue<TestItem> customTest(512U);
    EXPECT_GE(customTest.GetCapacity(), 512U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, HasCopyConstructor) {
    ShiftQueue<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftQueue<std::uint8_t> copy(test);

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

  TEST(ShiftQueueTest, HasMoveConstructor) {
    ShiftQueue<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftQueue<std::uint8_t> moved(std::move(test));

    EXPECT_EQ(moved.Count(), 10U);

    std::uint8_t retrieved[10];
    moved.Read(retrieved, 10);

    EXPECT_EQ(moved.Count(), 0U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ItemsCanBeAppended) {
    ShiftQueue<std::uint8_t> test;

    std::uint8_t items[128];
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ItemsCanBeAppendedWithMoveSemantics) {
    ShiftQueue<std::uint8_t> test;

    std::uint8_t items[128];
    test.Shove(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ItemsCanBeReadAndWritten) {
    ShiftQueue<std::uint8_t> test;

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

  TEST(ShiftQueueTest, WritingInvokesCopyConstructor) {
    checkWritingInvokesCopyConstructor<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ShovingInvokesMoveConstructor) {
    checkShovingInvokesMoveConstructor<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, MoveSemanticsAreUsedWhenCapacityChanges) {
    checkMoveSemanticsAreUsedWhenCapacityChanges<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ReadUsesMoveSemanticsAndCallsDestructor) {
    checkReadUsesMoveSemanticsAndCallsDestructor<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, BufferDestroysLeftOverItemsWhenDestroyed) {
    checkBufferDestroysLeftOverItemsWhenDestroyed<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ExceptionDuringCapacityChangeCausesNoLeaks) {
    checkExceptionDuringCapacityChangeCausesNoLeaks<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ExceptionDuringWriteCausesNoLeaks) {
    checkExceptionDuringWriteCausesNoLeaks<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ExceptionDuringShoveCausesNoLeaks) {
    checkExceptionDuringShoveCausesNoLeaks<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftQueueTest, ExceptionDuringReadCausesNoLeaks) {
    checkExceptionDuringReadCausesNoLeaks<ShiftQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
