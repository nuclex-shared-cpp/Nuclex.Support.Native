#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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
#define NUCLEX_GEOMETRY_SOURCE 1

#include "Nuclex/Support/Services/ServiceContainer.h"

#include <gtest/gtest.h>

using namespace std;

namespace {

  // ------------------------------------------------------------------------------------------- //

  class CalculatorService {
    public: virtual int Add(int first, int second) = 0;
    public: virtual int Multiply(int first, int second) = 0;
  };

  class BrokenCalculator : public virtual CalculatorService {
    public: int Add(int first, int second) override { return first + second + 1; }
    public: int Multiply(int first, int second) override { return first + first * second; };
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      ServiceContainer test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, NewContainerHasNoServices) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ServicesCanBeAddedUnderOwnType) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0);
    test.Add(std::make_shared<BrokenCalculator>());
    EXPECT_EQ(test.CountServices(), 1);
    
    std::shared_ptr<CalculatorService> service;
    EXPECT_FALSE(test.TryGet<CalculatorService>(service));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ServiceContainerTest, ServicesCanBeAddedUnderServiceType) {
    ServiceContainer test;
    EXPECT_EQ(test.CountServices(), 0);
    test.Add<CalculatorService>(std::make_shared<BrokenCalculator>());
    EXPECT_EQ(test.CountServices(), 1);

    std::shared_ptr<CalculatorService> service;
    EXPECT_TRUE(test.TryGet<CalculatorService>(service));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
