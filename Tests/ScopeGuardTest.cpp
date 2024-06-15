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

#include "Nuclex/Support/ScopeGuard.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitMacroWorks) {
    bool wasCleanedUp = false;
    {
      ON_SCOPE_EXIT { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
    }

    EXPECT_TRUE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitTransactionMacroWorks) {
    bool wasCleanedUp = false;
    {
      auto cleanUp = ON_SCOPE_EXIT_TRANSACTION { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
    }

    EXPECT_TRUE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitTransactionCanBeCommitted) {
    bool wasCleanedUp = false;
    {
      auto cleanUp = ON_SCOPE_EXIT_TRANSACTION { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
      cleanUp.Commit();
    }

    EXPECT_FALSE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
