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

#ifndef NUCLEX_SUPPORT_VARIANT_H
#define NUCLEX_SUPPORT_VARIANT_H

#include <typeinfo>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps a value of an arbitrary type</summary>
  /// <remarks>
  ///   Whatever you do, only use this to interface with scripting languages or where
  ///   it really makes sense. C++ is not a dynamically typed language and a template-based
  ///   design is always a better choice. If you need to design interfaces that accept
  ///   arbitrary types, consider the <see cref="Any" /> class instead.
  /// </remarks>
  class Variant {


  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_VARIANT_H
