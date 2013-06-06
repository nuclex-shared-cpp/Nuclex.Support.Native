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

#ifndef NUCLEX_SUPPORT_CONFIG_H
#define NUCLEX_SUPPORT_CONFIG_H

// --------------------------------------------------------------------------------------------- //

/// \mainpage

// --------------------------------------------------------------------------------------------- //

// Platform recognition
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  #define NUCLEX_SUPPORT_WINRT 1
#elif defined(WIN32) || defined(_WIN32)
  #define NUCLEX_SUPPORT_WIN32 1
#else
  #define NUCLEX_SUPPORT_LINUX 1
#endif

// --------------------------------------------------------------------------------------------- //

// C++ language features
#if defined(_MSC_VER) && (_MSC_VER >= 1700) // Visual Studio 2012 has the C++11 features we use
  #define NUCLEX_SUPPORT_CXX11
#elif defined(__GNUG__) && ((__GNUC__ * 1000 * __GNUC_MINOR) >= 4007) // GCC 4.7 has, too
  #define NUCLEX_SUPPORT_CXX11
#else
  #define NUCLEX_SUPPORT_CXX03
#endif

// --------------------------------------------------------------------------------------------- //

// Endianness detection
#if defined(_MSC_VER) // MSVC is always little endian, including Windows on ARM
  #define NUCLEX_SUPPORT_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) // GCC
  #define NUCLEX_SUPPORT_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) // GCC
  #define NUCLEX_SUPPORT_BIG_ENDIAN
#else
  #error Could not determine whether platform is big or little endian
#endif

// --------------------------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (Nuclex.Storage.Native library). The NUCLEX_SUPPORT_SOURCE symbol is defined by
// all source files of the library, so you don't have to worry about a thing.
#if defined(_MSC_VER)

  #if defined(NUCLEX_SUPPORT_STATICLIB)
    #define NUCLEX_SUPPORT_API
  #else
    #if defined(NUCLEX_SUPPORT_SOURCE)
      // If we are building the DLL, export the symbols tagged like this
      #define NUCLEX_SUPPORT_API __declspec(dllexport)
    #else
      // If we are consuming the DLL, import the symbols tagged like this
      #define NUCLEX_SUPPORT_API __declspec(dllimport)
    #endif
  #endif

#elif defined(__GNUC__)

  #if defined(NUCLEX_SUPPORT_STATICLIB)
    #define NUCLEX_SUPPORT_API
  #else
    #if defined(NUCLEX_SUPPORT_SOURCE)
      #define NUCLEX_SUPPORT_API __attribute__ ((visibility ("default")))
    #else
      // If you use -fvisibility=hidden in GCC, exception handling and RTTI would break 
      // if visibility wasn't set during export _and_ import because GCC would immediately
      // forget all type infos encountered. See http://gcc.gnu.org/wiki/Visibility
      #define NUCLEX_SUPPORT_API __attribute__ ((visibility ("default")))
    #endif
  #endif

#else

  #error Unknown compiler, please implement shared library macros for your system

#endif

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_SUPPORT_CONFIG_H
