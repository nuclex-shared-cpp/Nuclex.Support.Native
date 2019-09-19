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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/StringConverter.h"

#include "Utf8/checked.h"

#include <vector>

namespace {

  /// <summary>Adds or removes decimal places from an already-formatted number</summary>
  /// <param name="text">Formatted number on which decimal places will be added or removed</param>
  /// <param name="minimumDecimalPlaces">Minimum number of decimal places to provide</param>
  /// <remarks>
  ///   Optimally, we would have a method that formats a floating point value up to
  ///   the requested length. Sadly, this means either using sprintf() which is a lot of
  ///   baggage and slow or using iostreams, which is even more baggage and even slower.
  ///   So adjusting an already-formatted number is the leanest way to solve this.
  /// </remarks>
  template<typename TString>
  void adjustDecimalPlaces(TString &text, typename TString::size_type minimumDecimalPlaces) {
    typename TString::size_type length = text.length();

    // Cut off unwanted zero characters after the decimal point
    typename TString::size_type decimalPointIndex = text.find_first_of('.');
    if(decimalPointIndex == std::string::npos) {
    } else {
      typename TString::size_type desiredLength = decimalPointIndex + minimumDecimalPlaces;
      typename TString::size_type lastIndex = length;
      while(lastIndex > desiredLength) {
        --lastIndex;
        if(text[lastIndex] != '0') {
          break; // Don't cut off
        }
      }

      // Cut off the unwanted zeros we found, if any
      if(lastIndex < length) {
        length = lastIndex + 1;
        text.erase(length);
      }

      // If we're under the desired length, append zeros to the end. We're sure that
      // there's a decimal point in the string at this point.
      if(length < desiredLength) {
        text.append(desiredLength - length + 1, '0');
      } else if(length > 0) {
        length -= 1;
        if(text[length] == '.') {
          text.erase(length); // Remove trailing decimal separator, if present
        }
      }
    }
  }

}

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::Utf16FromUtf8(const std::string &utf8String) {
    if(utf8String.empty()) {
      return std::wstring();
    }

    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<wchar_t> utf16Characters;
    utf16Characters.reserve(utf8String.length());

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to16(utf8String.begin(), utf8String.end(), std::back_inserter(utf16Characters));

    return std::wstring(&utf16Characters[0], utf16Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromUtf16(const std::wstring &utf16String) {
    if(utf16String.empty()) {
      return std::string();
    }

    // We guess that we need as many UTF-8 characters as we needed UTF-16 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char> utf8Characters;
    utf8Characters.reserve(utf16String.length());

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf16to8(
      utf16String.begin(), utf16String.end(), std::back_inserter(utf8Characters)
    );

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  std::string StringConverter::Utf8FromFloat(
    float value, std::size_t minimumDecimalPlaces /* = 0 */
  ) {
    
    std::string text = std::to_string(value);
    adjustDecimalPlaces(text, minimumDecimalPlaces);
    return text;
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::Utf16FromFloat(
    float value, std::size_t minimumDecimalPlaces /* = 0 */
  ) {
    std::wstring text = std::to_wstring(value);
    adjustDecimalPlaces(text, minimumDecimalPlaces);
    return text;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromDouble(
    double value, std::size_t minimumDecimalPlaces /* = 0 */
  ) {
    std::string text = std::to_string(value);
    adjustDecimalPlaces(text, minimumDecimalPlaces);
    return text;
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::Utf16FromDouble(
    double value, std::size_t minimumDecimalPlaces /* = 0 */
  ) {
    std::wstring text = std::to_wstring(value);
    adjustDecimalPlaces(text, minimumDecimalPlaces);
    return text;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
