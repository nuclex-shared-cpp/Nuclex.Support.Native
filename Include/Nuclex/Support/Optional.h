#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_OPTIONAL_H
#define NUCLEX_SUPPORT_OPTIONAL_H

#include "Nuclex/Support/Config.h"

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps a type and allows either the type or nothing to be passed</summary>
  /// <remarks>
  ///   This is similar to .NET's Nullable&lt;T&gt; type. It allows other types to be optionally
  ///   passed without having to create ugly out parameters or relying on pointers.
  /// </remarks>
  class Optional {


  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_OPTIONAL_H
