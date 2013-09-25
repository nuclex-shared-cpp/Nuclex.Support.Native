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
    char characters[_CVTBUFSIZE];
    return std::string(::_gcvt(from, 0, characters));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::string &from) {
    return std::stof(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const double &from) {
    char characters[_CVTBUFSIZE];
    return std::string(::_gcvt(from, 0, characters));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::string &from) {
    return std::stod(from);
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ITOA)
  template<> std::string lexical_cast<>(const int &from) {
    char characters[21]; // Max number of characters in exotic 64 bit integer platform
    return std::string(::itoa(from, characters, 10));
  }
#endif

  // ------------------------------------------------------------------------------------------- //

  template<> int lexical_cast<>(const std::string &from) {
    return std::stoi(from);
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ULTOA)
  template<> std::string lexical_cast<>(const unsigned long &from) {
    char characters[21];
    return std::string(::ultoa(from, characters, 10));
  }
#endif

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
    static const std::string falseString("true");

    if(from) {
      return trueString;
    } else {
      return falseString;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
