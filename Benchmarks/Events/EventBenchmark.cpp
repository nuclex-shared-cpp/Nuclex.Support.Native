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

  /// <summary>Dummy callback that does absolutely nothing</summary>
  /// <param name="value">
  ///   Value that will be processed in a way that prevents the compiler from optimizing
  ///   the entire call away
  /// </param>
  void doNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Another dummy callback that does absolutely nothing</summary>
  /// <param name="value">
  ///   Value that will be processed in a way that prevents the compiler from optimizing
  ///   the entire call away
  /// </param>
  void doMoreNothingCallback(int value) {
    celero::DoNotOptimizeAway(value);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Test fixture that takes care of subscribing and unsubscribing an event</summary>
  class Event2Fixture : public celero::TestFixture {

		/// <summary>Called before the benchmark runs to subscribe to an event</summary>
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }

		/// <summary>Called after the benchmark completes to unsubscribe from the event</summary>
    public: void tearDown() override {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }

    /// <summary>Test event that will have tw subscriptions</summary>
    protected: Nuclex::Support::Events::Event<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Test fixture that takes care of subscribing and unsubscribing an event</summary>
  class Event50Fixture : public celero::TestFixture {

		/// <summary>Called before the benchmark runs to subscribe to an event</summary>
		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }

		/// <summary>Called after the benchmark completes to unsubscribe from the event</summary>
    public: void tearDown() override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Unsubscribe<&doNothingCallback>();
      }
    }

    /// <summary>Test event that will have tw subscriptions</summary>
    protected: Nuclex::Support::Events::Event<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ConcurrentEvent2Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      this->testEvent.Subscribe<&doNothingCallback>();
      this->testEvent.Subscribe<&doMoreNothingCallback>();
    }

    public: void tearDown() override {
      this->testEvent.Unsubscribe<&doMoreNothingCallback>();
      this->testEvent.Unsubscribe<&doNothingCallback>();
    }

    protected: Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;

  };

  // ------------------------------------------------------------------------------------------- //

  class ConcurrentEvent50Fixture : public celero::TestFixture {

		public: void setUp(const celero::TestFixture::ExperimentValue &) override {
      for(std::size_t index = 0; index < 50; ++index) {
        this->testEvent.Subscribe<&doNothingCallback>();
      }
    }

    public: void tearDown() override {
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

  BASELINE(Subscribe2, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe2, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Subscribe50, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Subscribe50, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Unsubscribe2, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe2, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    testEvent.Subscribe<&doNothingCallback>();
    testEvent.Subscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doMoreNothingCallback>();
    testEvent.Unsubscribe<&doNothingCallback>();
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE(Unsubscribe50, NuclexEvent, 1000, 0) {
    Nuclex::Support::Events::Event<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Unsubscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK(Unsubscribe50, NuclexConcurrentEvent, 1000, 0) {
    Nuclex::Support::Events::ConcurrentEvent<void(int)> testEvent;
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Subscribe<&doNothingCallback>();
    }
    for(std::size_t index = 0; index < 50; ++index) {
      testEvent.Unsubscribe<&doNothingCallback>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Invoke2_x100, NuclexEvent, Event2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke2_x100, NuclexConcurrentEvent, ConcurrentEvent2Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BASELINE_F(Invoke50_x100, NuclexEvent, Event50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BENCHMARK_F(Invoke50_x100, NuclexConcurrentEvent, ConcurrentEvent50Fixture, 1000, 0) {
    for(std::size_t index = 0; index < 100; ++index) {
      this->testEvent.Emit(static_cast<int>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events
