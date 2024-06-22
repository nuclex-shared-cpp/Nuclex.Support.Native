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

#include "Nuclex/Support/Threading/StopSource.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(StopTokenTest, TriggerProvidesWatcher) {
    std::shared_ptr<StopSource> source = StopSource::Create();
    std::shared_ptr<const StopToken> token = source->GetToken();
    EXPECT_NE(token.get(), nullptr);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StopTokenTest, WatcherIsSignalledByTrigger) {
    std::shared_ptr<StopSource> source = StopSource::Create();
    std::shared_ptr<const StopToken> token = source->GetToken();
    EXPECT_FALSE(token->IsCanceled());
    source->Cancel();
    EXPECT_TRUE(token->IsCanceled());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StopTokenTest, WatcherCanLifePastTrigger) {
    std::shared_ptr<const StopToken> token;
    {
      std::shared_ptr<StopSource> source = StopSource::Create();
      token = source->GetToken();
      source->Cancel();
      source.reset();
    }

    EXPECT_TRUE(token->IsCanceled());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StopTokenTest, WatcherThrowsCanceledErrorWhenCanceled) {
    std::shared_ptr<StopSource> source = StopSource::Create();
    std::shared_ptr<const StopToken> token = source->GetToken();
    source->Cancel();

    EXPECT_THROW(
      token->ThrowIfCanceled(),
      Nuclex::Support::Errors::CanceledError
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StopTokenTest, CanceledErrorInheritsFutureError) {
    std::shared_ptr<StopSource> source = StopSource::Create();
    std::shared_ptr<const StopToken> token = source->GetToken();
    source->Cancel();

    EXPECT_THROW(
      token->ThrowIfCanceled(),
      std::future_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
