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

#include "./NumberFormatter.h"
#include "./DragonBox-1.1.2/dragonbox.h" // for the float-to-decimal algorithm

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [46] */, float value) {
    jkj::dragonbox::float_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      float, jkj::dragonbox::default_float_traits<float>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<float>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          float, jkj::dragonbox::default_float_traits<float>
        >(significandBits, exponentBits);

        // TODO: Convert result into string
        return FormatInteger(buffer, result.significand);

      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else {
      if(significandBits.has_all_zero_significand_bits()) {
        if(significandBits.is_negative()) {
          std::memcpy(buffer, "-Infinity", 9);
          return buffer + 9;
        } else {
          std::memcpy(buffer, "Infinity", 8);
          return buffer + 8;
        }
      } else {
        std::memcpy(buffer, "NaN", 3);
        return buffer + 3;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  char *FormatFloat(char *buffer /* [325] */, double value) {
    jkj::dragonbox::float_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > floatBits(value);

    unsigned int exponentBits = floatBits.extract_exponent_bits();

    jkj::dragonbox::signed_significand_bits<
      double, jkj::dragonbox::default_float_traits<double>
    > significandBits = floatBits.remove_exponent_bits(exponentBits);

    if(floatBits.is_finite(exponentBits)) {
      if(significandBits.is_negative()) {
        *buffer = '-';
        ++buffer;
      }
      if(floatBits.is_nonzero()) {
        jkj::dragonbox::decimal_fp<
          typename jkj::dragonbox::default_float_traits<double>::carrier_uint,
          true, // return has a sign bit
          false // don't care about trailing zeros
        > result = jkj::dragonbox::to_decimal<
          double, jkj::dragonbox::default_float_traits<double>
        >(significandBits, exponentBits);

        // TODO: Convert result into string
        return FormatInteger(buffer, result.significand);

      } else {
        std::memcpy(buffer, "0.0", 3);
        return buffer + 3;
      }

      return buffer;

    } else {
      if(significandBits.has_all_zero_significand_bits()) {
        if(significandBits.is_negative()) {
          std::memcpy(buffer, "-Infinity", 9);
          return buffer + 9;
        } else {
          std::memcpy(buffer, "Infinity", 8);
          return buffer + 8;
        }
      } else {
        std::memcpy(buffer, "NaN", 3);
        return buffer + 3;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
