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

#include "Nuclex/Support/Config.h"

#define NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H
#include "Nuclex/Support/Services2/Private/IsSharedPtr.inl"
#include "Nuclex/Support/Services2/Private/IsInjectableType.inl"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock interface to unit test the IsInjectableType template</summary>
  class AbstractInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~AbstractInterface() = default;

    /// <summary>Mock of a pure virtual method that is exactly what it says</summary>
    public: virtual void PureVirtualMethod() = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of an abstract service interface for testing</summary>
  class Implementation : public AbstractInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: Implementation() {}
    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: ~Implementation() override = default;

    /// <summary>
    ///   Empty implementation of the pure virtual method from the service interface
    /// </summary>
    public: void PureVirtualMethod() {}

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services2::Private {

  // ------------------------------------------------------------------------------------------- //

  TEST(IsInjectableTypeTest, BasicTypesAreNotInjectable) {
    EXPECT_FALSE(IsInjectableType<int>::value);
    EXPECT_FALSE(IsInjectableType<float>::value);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IsInjectableTypeTest, NonSharedPtrsAreNotInjectable) {
    EXPECT_FALSE(IsInjectableType<AbstractInterface>::value);
    EXPECT_FALSE(IsInjectableType<Implementation>::value);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IsInjectableTypeTest, SharedPtrToConcreteClassIsInjectable) {
    EXPECT_TRUE(IsInjectableType<std::shared_ptr<Implementation>>::value);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IsInjectableTypeTest, SharedPtrToAbstractClassIsInjectable) {
    EXPECT_TRUE(IsInjectableType<std::shared_ptr<AbstractInterface>>::value);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
