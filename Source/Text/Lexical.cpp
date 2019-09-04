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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/Lexical.h"

#include <cstdlib>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const float &from) {
#if defined(HAVE_GCVT)
    char characters[_CVTBUFSIZE];
    #pragma warning(push)
    #pragma warning(disable:4996) // Use _gcvt_s() - ignored, _CVTBUFSIZE covers any integer
    return std::string(::_gcvt(from, 0, characters));
    #pragma warning(pop)
#else
    return std::to_string(from);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::string &from) {
    return std::stof(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const double &from) {
#if defined(HAVE_GCVT)
    char characters[_CVTBUFSIZE];
    #pragma warning(push)
    #pragma warning(disable:4996) // Use _gcvt_s() - ignored, _CVTBUFSIZE covers any integer
    return std::string(::_gcvt(from, 0, characters));
    #pragma warning(pop)
#else
    return std::to_string(from);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::string &from) {
    return std::stod(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const int &from) {
#if defined(HAVE_ITOA)
    char characters[21]; // Max number of characters in exotic 64 bit integer platform
    #pragma warning(push)
    #pragma warning(disable:4996) // Use _gcvt_s() - ignored, _CVTBUFSIZE covers any integer
    return std::string(::_itoa(from, characters, 10));
    #pragma warning(pop)
#else
    return std::to_string(from);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> int lexical_cast<>(const std::string &from) {
    return std::stoi(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const unsigned long &from) {
#if defined(HAVE_ULTOA)
    char characters[21];
    #pragma warning(push)
    #pragma warning(disable:4996) // Use _gcvt_s() - ignored, _CVTBUFSIZE covers any integer
    return std::string(::_ultoa(from, characters, 10));
    #pragma warning(pop)
#else
    return std::to_string(from);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const std::string &from) {
    if(from.length() == 4) {
      static const char lowerChars[] = { 't', 'r', 'u', 'e' };
      static const char upperChars[] = { 'T', 'R', 'U', 'E' };

      for(std::size_t index = 0; index < 4; ++index) {
        if((from[index] != lowerChars[index]) && (from[index] != upperChars[index])) {
          return false;
        }
      }

      return true;
    }
    
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const bool &from) {
    static const std::string trueString("true");
    static const std::string falseString("false");

    if(from) {
      return trueString;
    } else {
      return falseString;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
