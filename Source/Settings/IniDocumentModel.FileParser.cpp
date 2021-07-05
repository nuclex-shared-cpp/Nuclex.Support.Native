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

#include "IniDocumentModel.FileParser.h"

#include <memory> // for std::unique_ptr, std::align()
#include <type_traits> // for std::is_base_of
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

// Ambiguous cases and their resolution:
//
//   ["Hello]"       -> Malformed
//   [World          -> Malformed
//   [Foo] = Bar     -> Assignment, no section
//   [Woop][Woop]    -> Two sections, one w/newline one w/o
//   [Foo] Bar = Baz -> Section and assignment
//   [[Yay]          -> Malformed, section
//

// Allocation schemes:
//
//   By line                      -> lots of micro-allocations
//   In blocks (custom allocator) -> I have to do reference counting to free anything
//   Load pre-alloc, then by line -> Fast for typical case, no or few micro-allocations
//                                   But requires pre-scan of entire file + more code

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified character is a whiteapce</summary>
  /// <param name="utf8SingleByteCharacter">
  ///   Character the will be checked for being a whitespace
  /// </param>
  /// <returns>True if the character was a whitespace, false otherwise</returns>
  bool isWhitepace(std::uint8_t utf8SingleByteCharacter) {
    return (
      (utf8SingleByteCharacter == ' ') ||
      (utf8SingleByteCharacter == '\t') ||
      (utf8SingleByteCharacter == '\r') ||
      (utf8SingleByteCharacter == '\n')
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::FileParser::FileParser(
    const std::uint8_t *fileContents, std::size_t byteCount
  ) :
    fileBegin(fileContents),
    fileEnd(fileContents + byteCount) {}

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::SetDocumentModel(IniDocumentModel *documentModel) {
    this->target = documentModel;
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::Parse() {
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
