#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2012 Nuclex Development Labs

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

#include "Nuclex/Support/Text/StringConverter.h"
#include "Utf8/checked.h"
#include <vector>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::WideCharFromUtf8(const std::string &utf8String) {
    if(utf8String.empty()) {
      return std::wstring();
    }

    // We guess that we need as many wide characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<wchar_t> wideCharacters;
    wideCharacters.reserve(utf8String.length());

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to16(utf8String.begin(), utf8String.end(), std::back_inserter(wideCharacters));

    return std::wstring(&wideCharacters[0], wideCharacters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromWideChar(const std::wstring &wideCharString) {
    if(wideCharString.empty()) {
      return std::string();
    }

    // We guess that we need as many UTF-8 characters as we needed wide characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char> utf8Characters;
    utf8Characters.reserve(wideCharString.length());

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf16to8(
      wideCharString.begin(), wideCharString.end(), std::back_inserter(utf8Characters)
    );

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers
