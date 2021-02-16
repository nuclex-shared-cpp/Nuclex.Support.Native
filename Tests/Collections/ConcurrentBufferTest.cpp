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

#include "ConcurrentBufferTest.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of bits in the specified data type</summary>
  /// <typeparam name="TValue">Data type whose bits will be counted</typeparam>
  /// <returns>The number of bits in the specified data type</returns>
  template<typename TValue>
  inline constexpr std::size_t CountBits() {
    return sizeof(TValue) * 8U;
  }

  /// <summary>Counts the number of bits in a 16 bit integer</summary>
  /// <returns>The value 16</returns>
  template<>
  inline constexpr std::size_t CountBits<std::uint16_t>() { return 16U; }

  /// <summary>Counts the number of bits in a 32 bit integer</summary>
  /// <returns>The value 32</returns>
  template<>
  inline constexpr std::size_t CountBits<std::uint32_t>() { return 32U; }

  /// <summary>Counts the number of bits in a 64 bit integer</summary>
  /// <returns>The value 64</returns>
  template<>
  inline constexpr std::size_t CountBits<std::uint64_t>() { return 64U; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Forms a bit mask where the specific number of bits are set</summary>
  /// <param name="lowestBitIndex">Zero-based index of the lowest bit that will be set</param>
  /// <param name="bitCount">Number of bits that will be set</param>
  /// <returns>A bit mask with the specified range of bits set
  std::size_t BitMask(std::size_t lowestBitIndex, std::size_t bitCount) {
    return (
      (static_cast<std::size_t>(-1) << static_cast<int>(lowestBitIndex + bitCount)) ^
      (static_cast<std::size_t>(-1) << static_cast<int>(lowestBitIndex))
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  std::size_t HighContentionBufferTest::getBitMaskForThreadCount(std::size_t threadCount) {
    return BitMask(0, threadCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, BitMaskIsCalculatedCorrectly) {
    EXPECT_EQ(BitMask(0, 1), 1U);
    EXPECT_EQ(BitMask(0, 2), 3U);
    EXPECT_EQ(BitMask(0, 3), 7U);
    EXPECT_EQ(BitMask(0, 4), 15U);
    EXPECT_EQ(BitMask(0, 5), 31U);
    EXPECT_EQ(BitMask(0, 6), 63U);
    EXPECT_EQ(BitMask(0, 7), 127U);
    EXPECT_EQ(BitMask(0, 8), 255U);
    EXPECT_EQ(BitMask(0, 9), 511U);
    EXPECT_EQ(BitMask(0, 10), 1023U);
    EXPECT_EQ(BitMask(0, 11), 2047U);
    EXPECT_EQ(BitMask(0, 12), 4095U);
    EXPECT_EQ(BitMask(0, 13), 8191U);
    EXPECT_EQ(BitMask(0, 14), 16383U);
    EXPECT_EQ(BitMask(0, 15), 32767U);
    EXPECT_EQ(BitMask(0, 16), 65535U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpOneThread) {
    HighContentionBufferTest oneThread(1);
    oneThread.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpTwoThreads) {
    HighContentionBufferTest twoThreads(2);
    twoThreads.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, CanSpinUpFourThreads) {
    HighContentionBufferTest fourThreads(4);
    fourThreads.StartThreads();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HighContentionBufferTestTest, AllThreadsActuallyRun) {
    class ThreadCountingTest : public HighContentionBufferTest {
      public: ThreadCountingTest() :
        HighContentionBufferTest(4),
        executedThreadCount(0) {}

      public: std::size_t CountExecutedThreads() const {
        return this->executedThreadCount.load(std::memory_order_acquire);
      }

      protected: void Thread(std::size_t) override {
        this->executedThreadCount.fetch_add(1, std::memory_order_relaxed);
      }

      private: std::atomic<std::size_t> executedThreadCount;
    };

    ThreadCountingTest fourThreads;
    fourThreads.StartThreads();
    fourThreads.JoinThreads();
    EXPECT_EQ(fourThreads.CountExecutedThreads(), 4U);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
