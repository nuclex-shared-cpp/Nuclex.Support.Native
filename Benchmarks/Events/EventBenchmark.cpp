#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Event.h"
#include "Nuclex/Support/Events/ConcurrentEvent.h"

#include <celero/Celero.h>

#include <algorithm> // for std::copy_n()
#include <random> // for std::mt19937
#include <cstdint> // for std::uint32_t, std::uint64_t
#include <string> // for std::string
#include <type_traits> // for std::is_signed
#include <cmath> // for std::abs()

namespace {

  // ------------------------------------------------------------------------------------------- //

  void doNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void doMoreNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  class NuclexEventFixture : public celero::TestFixture {
    protected: Nuclex::Support::Events::Event<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  class NuclexConcurrentEventFixture : public celero::TestFixture {
    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  class NuclexEventUnsubscribe2Fixture : public celero::TestFixture {
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }
    public: void Unsubscribe() {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }
    protected: Nuclex::Support::Events::Event<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  class NuclexConcurrentEventUnsubscribe2Fixture : public celero::TestFixture {
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }
    public: void Unsubscribe() {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }
    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  class NuclexEventUnsubscribe50Fixture : public celero::TestFixture {
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }
    public: void Unsubscribe() {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Unsubscribe<&doNothingCallback>();
      }
    }
    protected: Nuclex::Support::Events::Event<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  class NuclexConcurrentEventUnsubscribe50Fixture : public celero::TestFixture {
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }
    public: void Unsubscribe() {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Unsubscribe<&doNothingCallback>();
      }
    }
    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fast random number generator used in the benchmark</summary>
  std::mt19937_64 randomNumberGenerator;
  /// <summary>Uniform distribution to make the output cover all possible integers</summary>
  std::uniform_int_distribution<std::uint32_t> randomNumberDistribution;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Subscribe2, NuclexEvent, NuclexEventFixture, 30, 100000) {
    this->testEvent.Subscribe<&doNothingCallback>();
    this->testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Subscribe2, NuclexConcurrentEvent, NuclexConcurrentEventFixture, 30, 10000) {
    this->testEvent.Subscribe<&doNothingCallback>();
    this->testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Subscribe50, NuclexEvent, NuclexEventFixture, 30, 100000) {
    for(std::size_t index = 0; index < 50; ++index) {
      this->testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Subscribe50, NuclexConcurrentEvent, NuclexConcurrentEventFixture, 10, 250) {
    for(std::size_t index = 0; index < 50; ++index) {
      this->testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Unsubscribe2, NuclexEvent, NuclexEventUnsubscribe2Fixture, 30, 100000) {
    Unsubscribe();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Unsubscribe2, NuclexConcurrentEvent, NuclexConcurrentEventUnsubscribe2Fixture, 30, 1000) {
    Unsubscribe();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Unsubscribe50, NuclexEvent, NuclexEventUnsubscribe50Fixture, 30, 10000) {
    Unsubscribe();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Unsubscribe50, NuclexConcurrentEvent, NuclexConcurrentEventUnsubscribe50Fixture, 10, 250) {
    Unsubscribe();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
