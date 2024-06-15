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

#include "Nuclex/Support/Optional.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class that sets an external boolean when an instance is destroyed</summary>
  class DestructionSignaller {

    /// <summary>Initializes a new destruction signalling helper</summary>
    /// <param name="booleanToSet">Boolean that will be set upon destruction</param>
    public: DestructionSignaller(bool *booleanToSet) :
      booleanToSet(booleanToSet) {}

    /// <summary>Constructs a destruction signaller by copying another instance</summary>
    /// <param name="other">Other destruction signaller that will be copied</parma>
    public: DestructionSignaller(const DestructionSignaller &other) = default;

    /// <summary>Constructs a destruction signaller taking over another instance</summary>
    /// <param name="other">Other destruction signaller that will be taken over</parma>
    public: DestructionSignaller(DestructionSignaller &&other) :
      booleanToSet(other.booleanToSet) {
      other.booleanToSet = nullptr;
    }

    /// <summary>Sets the boolean to indicate destruction unless disarmed</summary>
    public: ~DestructionSignaller() {
      if(this->booleanToSet != nullptr) {
        *this->booleanToSet = true;
      }
    }

    /// <summary>Disarms the signaller in case the external boolean is no longer valid</summary>
    public: void Disarm() {
      this->booleanToSet = nullptr;
    }

    /// <summary>Copies another instance of the destruction signaller</summary>
    /// <param name="other">Other destruction signaller that will be copied</parma>
    /// <returns>This instance after the other destruction signaller was copied</returns>
    public: DestructionSignaller &operator =(const DestructionSignaller &other) = default;

    /// <summary>Takes over another instance of the destruction signaller</summary>
    /// <param name="other">Other destruction signaller that will be taken over</parma>
    /// <returns>This instance after the other destruction signaller was taken over</returns>
    public: DestructionSignaller &operator =(DestructionSignaller &&other) {
      this->booleanToSet = other.booleanToSet;
      other.booleanToSet = nullptr;
      return *this;
    }

    /// <summary>Pointer to an external boolean that will be set</summary>
    private: bool *booleanToSet;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      Optional<int> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Optional<int> test(12345);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, HasCopyConstructor) {
    Optional<int> original(12345);
    Optional<int> copy(original);

    EXPECT_EQ(copy.Get(), 12345);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, HasMoveConstructor) {
    Optional<int> original(12345);
    Optional<int> copy(std::move(original));

    EXPECT_FALSE(original.HasValue());
    EXPECT_EQ(copy.Get(), 12345);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, CanBeReset) {
    Optional<int> test(12345);
    EXPECT_TRUE(test.HasValue());
    test.Reset();
    EXPECT_FALSE(test.HasValue());

    EXPECT_NO_THROW(
      test.Reset();
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, CanBeCopyAssigned) {
    bool copiedOverInstanceWasDestroyed = false;
    {
      DestructionSignaller signaller(&copiedOverInstanceWasDestroyed);
      Optional<DestructionSignaller> test(signaller); // Copies the signaller
      signaller.Disarm(); // Disarm the in-scope one

      // The Any should have made a single copy of the signaller, so at this point,
      // no instances of the signaller should have been destroyed yet
      EXPECT_FALSE(copiedOverInstanceWasDestroyed);

      // Overwrite the instance with another 'Any' value, destroying its earlier contents
      DestructionSignaller dummySignaller(nullptr);
      Optional<DestructionSignaller> other(dummySignaller);
      test = other;

      // Now the signaller within the first any should have been destroyed
      EXPECT_TRUE(copiedOverInstanceWasDestroyed);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalTest, CanBeMoveAssigned) {
    bool copiedOverInstanceWasDestroyed = false;
    {
      DestructionSignaller signaller(&copiedOverInstanceWasDestroyed);
      Optional<DestructionSignaller> test(signaller); // Copies the signaller
      signaller.Disarm(); // Disarm the in-scope one

      // The Any should have made a single copy of the signaller, so at this point,
      // no instances of the signaller should have been destroyed yet
      EXPECT_FALSE(copiedOverInstanceWasDestroyed);

      // Overwrite the instance with another 'Any' value, destroying its earlier contents
      Optional<DestructionSignaller> other(signaller); // Construct with disarmed signaller
      other = std::move(test); // Signaller should be moved, no copied & destroyed

      EXPECT_FALSE(copiedOverInstanceWasDestroyed);

      DestructionSignaller dummySignaller(nullptr);
      test = dummySignaller; // Should no longer own the signaller

      // Since the signaller was moved rather than copied, no instance of it
      // should be destroyed at this point
      EXPECT_FALSE(copiedOverInstanceWasDestroyed);

      other = dummySignaller;

      // Now the signaller should have been destroyed
      EXPECT_TRUE(copiedOverInstanceWasDestroyed);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
