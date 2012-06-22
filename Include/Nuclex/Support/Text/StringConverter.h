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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
#define NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H

#include "../Config.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helps converting strings between different formats</summary>
  class StringConverter {

    /// <summary>Converts an UTF-8 string into a wide char string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A wide char version of the provided ansi string</returns>
    public: NUCLEX_SUPPORT_API static std::wstring WideCharFromUtf8(
      const std::string &utf8String
    );

    /// <summary>Converts a wide char string into an UTF-8 string</summary>
    /// <param name="wideCharString">Wide char string that will be converted</param>
    /// <returns>An UTF-8 version of the provided wide char string</returns>
    public: NUCLEX_SUPPORT_API static std::string Utf8FromWideChar(
      const std::wstring &wideCharString
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
