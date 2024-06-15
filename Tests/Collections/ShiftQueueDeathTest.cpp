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
#include <gtest/gtest.h>

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(ShiftQueueDeathTest, SkippingOnEmptyBufferTriggersAssertion) {
    ShiftQueue<std::uint8_t> test;

    ASSERT_DEATH(
      test.Skip(1),
      u8".*Amount of data skipped must be less or equal to the amount of data in the buffer.*"
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NDEBUG)
  TEST(ShiftQueueDeathTest, ReadingFromEmptyBufferTriggersAssertion) {
    ShiftQueue<std::uint8_t> test;

    std::uint8_t retrieved[1];

    ASSERT_DEATH(
      test.Read(retrieved, 1),
      u8".*Amount of data read must be less or equal to the amount of data in the buffer.*"
    );
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
