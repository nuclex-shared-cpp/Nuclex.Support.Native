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

#include "Nuclex/Support/Services2/StandardServiceCollection.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock interface to unit test the ServiceCollection class</summary>
  class AbstractInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~AbstractInterface() = default;

    /// <summary>Mock of a pure virtual method that is exactly what it says</summary>
    public: virtual void ExampleMethod() = 0;

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
    public: void ExampleMethod() override {}

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services2::Private {

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, SingletonImplementationClassCanBeService) {
    StandardServiceCollection services;

    services.AddSingleton<Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, SingletonServiceCanHaveSeparateImplementation) {
    StandardServiceCollection services;

    services.AddSingleton<AbstractInterface, Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, SingletonServiceCanUseFactoryFunction) {
    StandardServiceCollection services;

    services.AddSingleton<AbstractInterface>(
      [](const std::shared_ptr<ServiceProvider> &) -> std::shared_ptr<AbstractInterface> {
        return std::make_shared<Implementation>();
      }
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, SingletonServiceCanServeExistingInstance) {
    StandardServiceCollection services;

    services.AddSingleton<AbstractInterface>(std::make_shared<Implementation>());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, ScopedImplementationClassCanBeService) {
    StandardServiceCollection services;

    services.AddScoped<Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, ScopedServiceCanHaveSeparateImplementation) {
    StandardServiceCollection services;

    services.AddScoped<AbstractInterface, Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, ScopedServiceCanUseFactoryFunction) {
    StandardServiceCollection services;

    services.AddScoped<AbstractInterface>(
      [](const std::shared_ptr<ServiceProvider> &) -> std::shared_ptr<AbstractInterface> {
        return std::make_shared<Implementation>();
      }
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, ScopedServiceCanServeExistingInstance) {
    StandardServiceCollection services;

    services.AddScoped<AbstractInterface>(std::make_shared<Implementation>());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, TransientImplementationClassCanBeService) {
    StandardServiceCollection services;

    services.AddTransient<Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, TransientServiceCanHaveSeparateImplementation) {
    StandardServiceCollection services;

    services.AddTransient<AbstractInterface, Implementation>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, TransientServiceCanUseFactoryFunction) {
    StandardServiceCollection services;

    services.AddTransient<AbstractInterface>(
      [](const std::shared_ptr<ServiceProvider> &) -> std::shared_ptr<AbstractInterface> {
        return std::make_shared<Implementation>();
      }
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, TransientServiceCanServeExistingInstance) {
    StandardServiceCollection services;

    services.AddTransient<AbstractInterface>(std::make_shared<Implementation>());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceCollectionTest, CanCreateServiceProvider) {
    StandardServiceCollection services;
    services.AddSingleton<AbstractInterface, Implementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    EXPECT_TRUE(static_cast<bool>(sp));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
