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

#include "../../Source/Settings/IniDocumentModel.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An average .ini file without any special or ambiguous contents</summary>
  const char VanillaIniFile[] =
    "GlobalProperty=1\n"
    "\n"
    "[ImportantStuff]\n"
    ";CommentedOut=5000\n"
    "Normal=42\n"
    "\n";

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(IniDocumentModelTest, HasFileContentsConstructor) {
    EXPECT_NO_THROW(
      IniDocumentModel dom(
        reinterpret_cast<const std::uint8_t *>(VanillaIniFile),
        sizeof(VanillaIniFile) - 1
      );
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
