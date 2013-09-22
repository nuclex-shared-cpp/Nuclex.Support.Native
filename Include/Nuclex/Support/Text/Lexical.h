#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_TEXT_LEXICAL_H
#define NUCLEX_SUPPORT_TEXT_LEXICAL_H

#include "Nuclex/Support/Config.h"

#include <sstream>
#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  template<typename TTarget, typename TSource>
  inline TTarget lexical_cast(const TSource &from) {
    std::stringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    return to;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a wide string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  template<typename TTarget, typename TSource>
  inline TTarget wlexical_cast(const TSource &from) {
    std::wstringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    return to;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into a string</summary>
  /// <param name="from">Floating point value that will be converted</param>
  /// <returns>A string containing the printed floating point value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const float &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API float lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a double precision floating point value into a string</summary>
  /// <param name="from">Double precision Floating point value that will be converted</param>
  /// <returns>A string containing the printed double precision floating point value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const double &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API double lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ITOA)
  /// <summary>Converts an integer value into a string</summary>
  /// <param name="from">Integer value that will be converted</param>
  /// <returns>A string containing the printed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const int &from);
#endif

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ATOI)
  /// <summary>Converts a string into an integer value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The integer value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API int lexical_cast<>(const std::string &from);
#endif

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LEXICAL_H
