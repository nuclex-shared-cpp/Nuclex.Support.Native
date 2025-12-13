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

  /// <summary>Mock interface to unit test the ServiceProvider class</summary>
  class PrintInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~PrintInterface() = default;

    /// <summary>Mock of a pure virtual method that 'prints' a message</summary>
    /// <param name="message">Message that should be printed</param>
    public: virtual void Print(const std::u8string &message) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of an abstract print interface for testing</summary>
  class PrintImplementation : public PrintInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: PrintImplementation() : LastPrintedMessage() {}
    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: ~PrintImplementation() override = default;

    /// <summary>Mock of a pure virtual method that 'prints' a message</summary>
    /// <param name="message">Message that should be printed</param>
    public: virtual void Print(const std::u8string &message) override {
      this->LastPrintedMessage = message;
    }

    /// <summary>Most recent message passed to the print method</summary>
    public: std::u8string LastPrintedMessage;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock interface to unit test the ServiceProvider class</summary>
  class GreeterInterface {

    /// <summary>Empty destructor because the mock has no members and does nothing</summary>
    public: virtual ~GreeterInterface() = default;

    /// <summary>Mock of a pure virtual method that fetches a name</summary>
    /// <returns>The name that has been fetched</returns>
    public: virtual const std::u8string &GetName() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock implementation of an abstract name interface for testing</summary>
  class GreeterImplementation : public GreeterInterface {

    /// <summary>Initializes the mock implementation (does nothing)</summary>
    public: GreeterImplementation(const std::shared_ptr<PrintInterface> &printer) :
      printer(printer) {}

    /// <summary>Destructor that releases the service reference again</summary>
    public: ~GreeterImplementation() override = default;

    /// <summary>Prints a test message using the dependency-injected printeR</summary>
    public: void DemandSurrender() {
      this->printer->Print(u8"All your base are belong to us");
    }

    /// <summary>Name that can be fetched</summary>
    private: std::shared_ptr<PrintInterface> printer;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services2::Private {

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceProviderTest, CanCreateServiceImplementation) {
    StandardServiceCollection services;
    services.AddSingleton<PrintImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<PrintImplementation> printer = sp->GetService<PrintImplementation>();
    ASSERT_TRUE(static_cast<bool>(printer));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceProviderTest, CanRequestServiceByInterface) {
    StandardServiceCollection services;
    services.AddSingleton<PrintInterface, PrintImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<PrintInterface> printer = sp->GetService<PrintInterface>();
    ASSERT_TRUE(static_cast<bool>(printer));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceProviderTest, CanCreateServiceImplementationWithDependencies) {
    StandardServiceCollection services;
    services.AddSingleton<PrintInterface, PrintImplementation>();
    services.AddSingleton<GreeterImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<GreeterImplementation> greeter = sp->GetService<GreeterImplementation>();
    ASSERT_TRUE(static_cast<bool>(greeter));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StandardServiceProviderTest, CanRequestServiceWithDependenciesByInterface) {
    StandardServiceCollection services;
    services.AddSingleton<PrintInterface, PrintImplementation>();
    services.AddSingleton<GreeterInterface, GreeterImplementation>();

    std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();
    ASSERT_TRUE(static_cast<bool>(sp));

    std::shared_ptr<GreeterInterface> greeter = sp->GetService<GreeterInterface>();
    ASSERT_TRUE(static_cast<bool>(greeter));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
