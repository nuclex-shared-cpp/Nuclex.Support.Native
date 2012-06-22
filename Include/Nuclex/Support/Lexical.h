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

#ifndef NUCLEX_SUPPORT_LEXICAL_H
#define NUCLEX_SUPPORT_LEXICAL_H

#include <sstream>
#include <string>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Extracts the value stored in the any</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  template<typename TTarget, typename TSource>
  TTarget lexical_cast(const TSource &from) {
    std::stringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    return to;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into a string</summary>
  /// <param name="from">Floating point value that will be converted</param>
  /// <returns>A string containing the printed floating point value</returns>
  template<> std::string lexical_cast<>(const float &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> float lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a double precision floating point value into a string</summary>
  /// <param name="from">Double precision Floating point value that will be converted</param>
  /// <returns>A string containing the printed double precision floating point value</returns>
  template<> std::string lexical_cast<>(const double &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> double lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_LEXICAL_H
