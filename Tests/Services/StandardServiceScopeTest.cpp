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

#include "Nuclex/Support/Services/StandardServiceCollection.h"
#include "Nuclex/Support/Services/ServiceScope.h"
#include "Nuclex/Support/Errors/UnresolvedDependencyError.h"
#include "Nuclex/Support/Errors/CyclicDependencyError.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Silly message the greeter can print</summary>
  const std::u8string SillyMessage(u8"All your base are belong to us");

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock interface to unit test the ServiceProvider class</summary>
  class PrintInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~PrintInterface() = default;

    /// <summary>Mock of a pure virtual method that 'prints' a message</summary>
    /// <param name="message">Message that should be printed</param>
    public: virtual void Print(const std::u8string &message) = 0;

    /// <summary>Fetches the last message the printer was asked to print</summary>
    /// <returns>A string containing the last printed message</returns>
    public: virtual const std::u8string &GetLastPrintedMessage() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of an abstract print interface for testing</summary>
  class PrintImplementation : public PrintInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: PrintImplementation() : lastPrintedMessage() {}
    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: ~PrintImplementation() override = default;

    /// <summary>Mock of a pure virtual method that 'prints' a message</summary>
    /// <param name="message">Message that should be printed</param>
    public: void Print(const std::u8string &message) override {
      this->lastPrintedMessage = message;
    }

    /// <summary>Fetches the last message the printer was asked to print</summary>
    /// <returns>A string containing the last printed message</returns>
    public: const std::u8string &GetLastPrintedMessage() const override {
      return this->lastPrintedMessage;
    }

    /// <summary>Most recent message passed to the print method</summary>
    private: std::u8string lastPrintedMessage;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock interface to unit test the ServiceProvider class</summary>
  class GreeterInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~GreeterInterface() = default;

    /// <summary>Prints a test message using the dependency-injected printer</summary>
    public: virtual void DemandSurrender() = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of an abstract name interface for testing</summary>
  class GreeterImplementation : public GreeterInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: GreeterImplementation(const std::shared_ptr<PrintInterface> &printer) :
      printer(printer) {}

    /// <summary>Destructor that releases the service reference again</summary>
    public: ~GreeterImplementation() override = default;

    /// <summary>Prints a test message using the dependency-injected printer</summary>
    public: void DemandSurrender() override {
      this->printer->Print(SillyMessage);
    }

    /// <summary>Name that can be fetched</summary>
    private: std::shared_ptr<PrintInterface> printer;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of that depends on its own service type</summary>
  class CyclicDependencyErrorGreeterImplementation : public GreeterInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: CyclicDependencyErrorGreeterImplementation(
      const std::shared_ptr<GreeterInterface> &
    ) {}

    /// <summary>Destructor that releases the service reference again</summary>
    public: ~CyclicDependencyErrorGreeterImplementation() override = default;

    /// <summary>Prints a test message using the dependency-injected printer</summary>
    public: void DemandSurrender() override {}

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services::Private {

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceScopeTest, ScopedServicesResolveOnlyInScope) {
    StandardServiceCollection services;
    services.AddScoped<PrintInterface, PrintImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<ServiceScope> sc = sp->CreateScope();
    ASSERT_TRUE(static_cast<bool>(sc));

    EXPECT_THROW(
      sp->GetService<PrintInterface>(),
      Errors::UnresolvedDependencyError
    );

    std::shared_ptr<PrintInterface> pi = sc->GetService<PrintInterface>();
    ASSERT_TRUE(static_cast<bool>(pi));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceScopeTest, ScopedServicesCanDependOnSingletonServices) {
    StandardServiceCollection services;
    services.AddSingleton<PrintInterface, PrintImplementation>();
    services.AddScoped<GreeterInterface, GreeterImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<ServiceScope> sc = sp->CreateScope();
    ASSERT_TRUE(static_cast<bool>(sc));
    std::shared_ptr<GreeterInterface> pi = sc->GetService<GreeterInterface>();
    ASSERT_TRUE(static_cast<bool>(pi));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceScopeTest, SingletonServicesMustNotDependOnScopedServices) {
    StandardServiceCollection services;
    services.AddScoped<PrintInterface, PrintImplementation>();
    services.AddSingleton<GreeterInterface, GreeterImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<ServiceScope> sc = sp->CreateScope();
    ASSERT_TRUE(static_cast<bool>(sc));

    EXPECT_THROW(
      sp->GetService<GreeterInterface>(),
      Errors::UnresolvedDependencyError
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services::Private
