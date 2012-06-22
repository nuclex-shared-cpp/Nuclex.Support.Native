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

#ifndef NUCLEX_SUPPORT_CONTAINERS_VARIANTTYPE_H
#define NUCLEX_SUPPORT_CONTAINERS_VARIANTTYPE_H

namespace Nuclex { namespace Support { namespace Containers {

  namespace VariantType {

    /// <summary>Different types that can be stored in a variant</summary>
    enum Enum {

      /// <summary>Nothing is being stored</summary>
      Empty = 0,
      /// <summary>A C++ boolean is being stored</summary>
      Boolean = 1,
      /// <summary>An unsigned 8 bit integer is being stored</summary>
      Uint8 = 2,
      /// <summary>A signed 8 bit integer is being stored</summary>
      Int8 = 2,
      /// <summary>An unsigned 16 bit integer is being stored</summary>
      Uint16 = 2,
      /// <summary>A signed 16 bit integer is being stored</summary>
      Int16 = 2,
      /// <summary>An unsigned 32 bit integer is being stored</summary>
      Uint32 = 2,
      /// <summary>A signed 32 bit integer is being stored</summary>
      Int32 = 2,
      /// <summary>An unsigned 64 bit integer is being stored</summary>
      Uint64 = 2,
      /// <summary>A signed 64 bit integer is being stored</summary>
      Int64 = 2,
      /// <summary>A floating point value is being stored</summary>
      Float = 2,
      /// <summary>A double precision floating point value is being stored</summary>
      Double = 2,
      /// <summary>A string is being stored</summary>
      String = 2,
      /// <summary>A wide character string is being stored</summary>
      WString = 2,
      /// <summary>A pointer to void is being stored</summary>
      VoidPointer = 2,
      /// <summary>An arbitrary type is being stored</summary>
      Any = 2,

    };

  } // namespace VariantType

}}} // namespace Nuclex::Support::Containers

#endif // NUCLEX_SUPPORT_CONTAINERS_VARIANTTYPE_H
