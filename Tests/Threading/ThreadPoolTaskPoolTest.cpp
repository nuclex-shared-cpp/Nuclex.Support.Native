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

#include "../Source/Threading/ThreadPoolTaskPool.h"

#include <memory> // for std::unique_ptr
#include <mutex> // for std::mutex

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock task used to test the task pool</summary>
  struct TestTask {

    /// <summary>Number of times a task constructor has been called</summary>
    public: static std::size_t ConstructorCallCount;
    /// <summary>Number of times a task destructor has been called</summary>
    public: static std::size_t DestructorCallCount;

    /// <summary>Initializes a new test task</summary>
    public: TestTask() {
      ++ConstructorCallCount;
    }

    /// <summary>Destroys a test task</summary>
    public: ~TestTask() {
      ++DestructorCallCount;
    }

    /// <summary>Size of the payload carried by the task</summary>
    public: std::size_t PayloadSize;
    /// <summary>Example content, never used, never accessed</summary>
    public: float Unused;
    /// <summary>Placeholder for the variable payload appended to the task</summary>
    public: std::uint8_t Payload[sizeof(std::uintptr_t)];

  };

  // ------------------------------------------------------------------------------------------- //

  std::size_t TestTask::ConstructorCallCount = 0;

  // ------------------------------------------------------------------------------------------- //

  std::size_t TestTask::DestructorCallCount = 0;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Used to avoid unit tests from interfering with each other in case they're run in parallel
  /// </summary>
  std::mutex CallCountMutex;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A pool of mock tasks</summary>
  typedef Nuclex::Support::Threading::ThreadPoolTaskPool<
    TestTask, offsetof(TestTask, Payload)
  > TestTaskPool;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TestTaskPool taskPool;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, TaskConstructorAndDestructorAreCalled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *myTask = taskPool.GetNewTask(32);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.DeleteTask(myTask);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, TasksCanBeRecycled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(32);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      TestTask *anotherTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      EXPECT_EQ(anotherTask, originalTask);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, RecycledTaskIsOnlyHandedOutWhenLargeEnough) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      TestTask *anotherTask = taskPool.GetNewTask(32);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 2);
      //EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      // Originally, I tested like this:
      // EXPECT_NE(anotherTask, originalTask);
      //
      // but GetNewTask() calls free upon encountering the 16 byte payload task,
      // and the C++ memory allocator then can allocate the 32 byte payload task
      // at the exact same memory address. This caused spurious failures.
      //
      EXPECT_EQ(anotherTask->PayloadSize, 32U);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, PoolDestructionKillsRecycledTasks) {
    std::lock_guard callCountScope(CallCountMutex);

    std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
    std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

    {
      TestTaskPool taskPool;

      TestTask *myTask = taskPool.GetNewTask(32);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(myTask);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);
    }

    EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
    EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, HugeTasksAreNotRecycled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(1024);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);

      TestTask *anotherTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 2);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);

      // Cannot do this, C++ allocator might (and does, in practice) hand out
      // the new 16 byte task at the same address as the freed 1024 byte task.
      //EXPECT_NE(anotherTask, originalTask);
      EXPECT_GE(anotherTask->PayloadSize, 16U);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Threading
