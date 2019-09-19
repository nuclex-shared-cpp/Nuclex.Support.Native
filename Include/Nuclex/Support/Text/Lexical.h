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

#ifndef NUCLEX_SUPPORT_TEXT_LEXICAL_H
#define NUCLEX_SUPPORT_TEXT_LEXICAL_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/StringConverter.h"

#include <string>

#if !defined(NUCLEX_SUPPORT_NO_IOSTREAMS)
#include <sstream>
#endif

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts from a string to a non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the string will be converted</typeparam>
  /// <param name="from">String that will be converted</param>
  /// <returns>The value converted to the specified string</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated and functions
  ///     such as gcvt() or itoa(). 
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. Primitive types can be converted without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget>
  inline TTarget lexical_cast(const char *from) {
#if defined(NUCLEX_SUPPORT_NO_IOSTREAMS)
    #error Unable to convert between these types without relying on iostreams
#else
    std::stringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    if(stringStream.fail() || stringStream.bad()) {
      std::string message("Could not convert from \"");
      stringStream >> message;
      message.append("\" (const cchar *) to (");
      message.append(typeid(TTarget).name());
      message.append(")");
      throw std::invalid_argument(message.c_str());
    }

    return to;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated and functions
  ///     such as gcvt() or itoa(). 
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. Primitive types can be converted without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget, typename TSource>
  inline TTarget lexical_cast(const TSource &from) {
#if defined(NUCLEX_SUPPORT_NO_IOSTREAMS)
    #error Unable to convert between these types without relying on iostreams
#else
    std::stringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    if(stringStream.fail() || stringStream.bad()) {
      std::string message("Could not convert from \"");
      stringStream >> message;
      message.append("\" (");
      message.append(typeid(TSource).name());
      message.append(") to (");
      message.append(typeid(TTarget).name());
      message.append(")");
      throw std::invalid_argument(message.c_str());
    }

    return to;
#endif
  }

  // ------------------------------------------------------------------------------------------- //
#if 1 // Wide character stuff is a dead-end idea of Win32, use StringConverter for this.
  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated and functions
  ///     such as gcvt() or itoa(). 
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. Primitive types can be converted without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget, typename TSource>
  inline TTarget wlexical_cast(const TSource &from) {
#if defined(NUCLEX_SUPPORT_NO_IOSTREAMS)
    #error Unable to convert between these types without relying on iostreams
#else
    std::wstringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    if(stringStream.fail() || stringStream.bad()) {
      std::string message("Could not convert from \"");
      {
        std::wstring value;
        stringStream >> value;
        message.append(StringConverter::Utf8FromUtf16(value));
      }
      message.append("\" (");
      message.append(typeid(TSource).name());
      message.append(") to (");
      message.append(typeid(TTarget).name());
      message.append(")");
      throw std::invalid_argument(message.c_str());
    }

    return to;
#endif
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a boolean value into a string</summary>
  /// <param name="from">Boolean value that will be converted</param>
  /// <returns>A string containing the printed boolean value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const bool &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API bool lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API bool lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 8 bit unsigned integer into a string</summary>
  /// <param name="from">8 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 8 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint8_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint8_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint8_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 8 bit unsigned integer into a string</summary>
  /// <param name="from">8 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 8 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int8_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int8_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int8_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 16 bit unsigned integer into a string</summary>
  /// <param name="from">16 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 16 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint16_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint16_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint16_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 16 bit signed integer into a string</summary>
  /// <param name="from">16 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 16 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int16_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int16_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int16_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 32 bit unsigned integer into a string</summary>
  /// <param name="from">32 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 32 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint32_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint32_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint32_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 32 bit signed integer into a string</summary>
  /// <param name="from">32 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 32 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int32_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int32_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int32_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 64 bit unsigned integer into a string</summary>
  /// <param name="from">64 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 64 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint64_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint64_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint64_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 64 bit signed integer into a string</summary>
  /// <param name="from">64 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 64 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int64_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int64_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int64_t lexical_cast<>(const std::string &from);

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

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LEXICAL_H
