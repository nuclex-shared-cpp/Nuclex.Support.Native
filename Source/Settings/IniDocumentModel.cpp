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

#include "IniDocumentModel.h"
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

  /// <summary>Determines the size of a type plus padding for another aligned member</summary>
  /// <typeparam name="T">Type whose size plus padding will be determined</typeparam>
  /// <returns>The size of the type plus padding with another aligned member</returns>
  template<typename T>
  constexpr std::size_t getSizePlusAlignmentPadding() {
    constexpr std::size_t misalignment = (sizeof(T) % alignof(T));
    if constexpr(misalignment > 0) {
      return sizeof(T) + (alignof(T) - misalignment);
    } else {
      return sizeof(T);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IniDocumentModel() :
    loadedLinesMemory(),
    createdLinesMemory(),
    firstLine(nullptr),
    sections(),
    hasSpacesAroundAssignment(true),
    hasEmptyLinesBetweenProperties(true) {

#if defined(NUCLEX_SUPPORT_WIN32)
    this->firstLine = allocateLine<Line>(reinterpret_cast<const std::uint8_t *>(u8"\r\n"), 2);
#else
    this->firstLine = allocateLine<Line>(reinterpret_cast<const std::uint8_t *>(u8"\n"), 1);
#endif
    this->firstLine->Previous = this->firstLine;
    this->firstLine->Next = this->firstLine;
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IniDocumentModel(const std::uint8_t *fileContents, std::size_t byteCount) :
    loadedLinesMemory(),
    createdLinesMemory(),
    firstLine(nullptr),
    sections(),
    hasSpacesAroundAssignment(true),
    hasEmptyLinesBetweenProperties(true) {
    parseFileContents(fileContents, byteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::~IniDocumentModel() {

    for(
      SectionMap::iterator iterator = this->sections.begin();
      iterator != this->sections.end();
      ++iterator
    ) {
      //delete iterator->second;
      iterator->second->~IndexedSection();
    }

    // If an existing .ini file was loaded, memory will have been allocated in chunks.
    for(
      std::vector<std::uint8_t *>::reverse_iterator iterator = this->loadedLinesMemory.rbegin();
      iterator != this->loadedLinesMemory.rend();
      ++iterator
    ) {
      delete[] *iterator;
    }

    // Delete the memory for any other lines that were created
    for(
      std::unordered_set<std::uint8_t *>::iterator iterator = this->createdLinesMemory.begin();
      iterator != this->createdLinesMemory.end();
      ++iterator
    ) {
      delete[] *iterator;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::parseFileContents(
    const std::uint8_t *fileContents, std::size_t byteCount
  ) {
    FileParser parser(fileContents, byteCount);
    parser.ParseInto(this);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  TLine *IniDocumentModel::allocateLine(const std::uint8_t *contents, std::size_t byteCount) {
    static_assert(std::is_base_of<Line, TLine>::value && u8"TLine inherits from Line");

    // Allocate memory for a new line, assign its content pointer to hold
    // the line loaded from the .ini file and copy the line contents into it.
    TLine *newLine = allocate<TLine>(byteCount);
    {
      newLine->Contents = (
        reinterpret_cast<std::uint8_t *>(newLine) + getSizePlusAlignmentPadding<TLine>()
      );
      newLine->Length = byteCount;

      std::copy_n(contents, byteCount, newLine->Contents);
    }

    return newLine;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename T>
  T *IniDocumentModel::allocate(std::size_t extraByteCount /* = 0 */) {

    // While we're asked to allocate a specific type, making extra bytes available
    // requires us to allocate as std::uint8_t. The start address still needs to be
    // appropriately aligned for the requested type (otherwise we'd have to keep
    // separate pointers for delete[] and for the allocated type).
    #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(T));
    #endif

    // Calculate the exact amount of memory required, including the extra bytes
    // aligned to the same conditions as the requested type.
    constexpr std::size_t requiredMemory = getSizePlusAlignmentPadding<T>();
    std::uint8_t *bytes = new std::uint8_t[requiredMemory + extraByteCount];
    this->createdLinesMemory.insert(bytes);

    return reinterpret_cast<T *>(bytes);

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
