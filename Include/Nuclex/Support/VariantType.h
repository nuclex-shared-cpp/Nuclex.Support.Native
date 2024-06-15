#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

#ifndef NUCLEX_SUPPORT_VARIANTTYPE_H
#define NUCLEX_SUPPORT_VARIANTTYPE_H

#include "Nuclex/Support/Config.h"

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Different types that can be stored in a variant</summary>
  enum class VariantType {

    /// <summary>Nothing is being stored</summary>
    Empty = 0,
    /// <summary>A C++ boolean is being stored</summary>
    Boolean = 1,
    /// <summary>An unsigned 8 bit integer is being stored</summary>
    Uint8 = 2,
    /// <summary>A signed 8 bit integer is being stored</summary>
    Int8 = 3,
    /// <summary>An unsigned 16 bit integer is being stored</summary>
    Uint16 = 4,
    /// <summary>A signed 16 bit integer is being stored</summary>
    Int16 = 5,
    /// <summary>An unsigned 32 bit integer is being stored</summary>
    Uint32 = 6,
    /// <summary>A signed 32 bit integer is being stored</summary>
    Int32 = 7,
    /// <summary>An unsigned 64 bit integer is being stored</summary>
    Uint64 = 8,
    /// <summary>A signed 64 bit integer is being stored</summary>
    Int64 = 9,
    /// <summary>A floating point value is being stored</summary>
    Float = 10,
    /// <summary>A double precision floating point value is being stored</summary>
    Double = 11,
    /// <summary>A string is being stored</summary>
    String = 12,
    /// <summary>A wide character string is being stored</summary>
    WString = 13,
    /// <summary>An arbitrary type is being stored</summary>
    Any = 14,
    /// <summary>A pointer to void is being stored</summary>
    VoidPointer = 15

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_VARIANTTYPE_H
