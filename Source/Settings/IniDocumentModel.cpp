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

#include <memory> // for std::unique_ptr
#include <type_traits> // for std::is_base_of
#include <algorithm> // for std::copy_n()

// Ambiguous cases and their resolution:
//
//   ["Hello]"       -> Bullshit
//   [World          -> Bullshit
//   [Foo] = Bar     -> Assignment, no section
//   [Woop][Woop]    -> Two sections, one w/newline one w/o
//   [Foo] Bar = Baz -> Section and assignment
//   [[Yay]          -> Bullshit, section

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Grows the specified byte count until hittint the required alignment</summary>
  /// <param name="byteCount">Byte counter that will be grown until aligned</param>
  /// <param name="alignment">Alignment the byte counter must have</param>
  void growUntilAligned(std::size_t &byteCount, std::size_t alignment) {
    std::size_t extraByteCount = byteCount % alignment;
    if(extraByteCount > 0) {
      byteCount += (alignment - extraByteCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Accumulates an estimate of the memory required to hold the document model
  /// </summary>
  class Nuclex::Support::Settings::IniDocumentModel::MemoryEstimator {

    /// <summary>Initializes a new memory estimator</summary>
    public: MemoryEstimator() :
      ByteCount(0),
      lineStartPosition(nullptr),
      sectionStarted(false),
      sectionEnded(false),
      foundAssignment(false),
      lineIsBullshit(false) {}

    /// <summary>Notifies the memory estimator that a new line has begun</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void BeginLine(const std::uint8_t *filePosition) {
      if(this->lineStartPosition != nullptr) {
        EndLine(filePosition);
      }

      this->lineStartPosition = filePosition;
    }

    /// <summary>Notifies the memory estimator that the current line has ended</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void EndLine(const std::uint8_t *filePosition) {
      if(this->lineIsBullshit) {
        growUntilAligned(this->ByteCount, alignof(Line));
        this->ByteCount += sizeof(Line[2]) / 2;
      } else if(this->foundAssignment) {
        growUntilAligned(this->ByteCount, alignof(PropertyLine));
        this->ByteCount += sizeof(PropertyLine[2]) / 2;
      } else if(this->sectionStarted && this->sectionEnded) {
        growUntilAligned(this->ByteCount, alignof(SectionLine));
        this->ByteCount += sizeof(SectionLine[2]) / 2;
      } else {
        growUntilAligned(this->ByteCount, alignof(Line));
        this->ByteCount += sizeof(Line[2]) / 2;
      }

      this->ByteCount += (filePosition - this->lineStartPosition);
      this->lineStartPosition = filePosition;

      this->sectionStarted = false;
      this->sectionEnded = false;
      this->foundAssignment =false;
      this->lineIsBullshit = false;
    }

    /// <summary>Notifies the memory estimator that a section has been opened</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void BeginSection(const std::uint8_t *filePosition) {
      if(this->sectionEnded) {
        EndLine(filePosition);
      }

      this->sectionStarted = true;
    }

    /// <summary>Notifies the memory estimator that a section has been closed</summary>
    public: void EndSection() {
      if(this->sectionStarted) {
        this->sectionEnded = true;
      }
    }

    /// <summary>Notifies the memory estimator that an equals sign has been found</summary>
    public: void AddAssignment() {
      if(this->foundAssignment) {
        this->lineIsBullshit = true;
      } else {
        this->foundAssignment = true;
      }
    }

    /// <summary>Number of bytes accumulated so far</summary>
    public: std::size_t ByteCount;
    /// <summary>File offset at which the current line begins</summary>
    private: const std::uint8_t *lineStartPosition;
    /// <summary>Whether a section start marker was encountered</summary>
    private: bool sectionStarted;
    /// <summary>Whether a section end marker was encountered</summary>
    private: bool sectionEnded;
    /// <summary>Whether an equals sign was encountered</summary>
    private: bool foundAssignment;
    /// <summary>Whether we a proof that the current line is malformed</summary>
    private: bool lineIsBullshit;

  };

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IniDocumentModel() :
    loadedLinesMemory(nullptr),
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
    loadedLinesMemory(nullptr),
    createdLinesMemory(),
    firstLine(nullptr),
    sections(),
    hasSpacesAroundAssignment(true),
    hasEmptyLinesBetweenProperties(true) {

    std::size_t requiredMemory = estimateRequiredMemory(fileContents, byteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::~IniDocumentModel() {
    for(
      std::unordered_set<std::uint8_t *>::iterator iterator = this->createdLinesMemory.begin();
      iterator != this->createdLinesMemory.end();
      ++iterator
    ) {
      delete[] *iterator;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t IniDocumentModel::estimateRequiredMemory(
    const std::uint8_t *fileContents, std::size_t byteCount
  ) {
    MemoryEstimator memoryEstimate;
    {
      bool previousWasNewLine = false;
      bool isInsideQuote = false;
      bool encounteredComment = false;

      // Make sure the memory estimator knows where the first line starts
      memoryEstimate.BeginLine(fileContents);

      // Go through the entire file contents byte-by-byte and do some basic parsing
      // to handle quotes and comments correctly. All of these characters are in
      // the ASCII range, thus there are no UTF-8 sequences that could be mistaken for
      // them (multi-byte UTF-8 codepoints will have the highest bit set in all bytes)
      const std::uint8_t *fileBegin = fileContents;
      const std::uint8_t *fileEnd = fileContents + byteCount;
      while(fileBegin < fileEnd) {
        std::uint8_t current = *fileBegin;
        previousWasNewLine = (current == '\n');

        // Newlines always reset the state, this parser does not support multi-line statements
        if(previousWasNewLine) {
          isInsideQuote = false;
          encounteredComment = false;
          memoryEstimate.EndLine(fileBegin + 1);
        } else if(isInsideQuote) { // Quotes make it ignore section and assignment characters
          if(current == '"') {
            isInsideQuote = false;
          }
        } else if(!encounteredComment) { // Otherwise, parse until comment encountered
          switch(current) {
            case ';':
            case '#': { encounteredComment = true; break; }
            case '[': { memoryEstimate.BeginSection(fileBegin); break; }
            case ']': { memoryEstimate.EndSection(); break; }
            case '=': { memoryEstimate.AddAssignment(); break; }
            case '"': { isInsideQuote = true; break; }
            default: { break; }
          }
        }

        ++fileBegin;
      } // while fileBegin < fileEnd

      // If the file didn't end with a line break, consider the contents of
      // the file's final line as another line
      if(!previousWasNewLine) {
        memoryEstimate.EndLine(fileEnd);
      }
    } // beauty scope

    return memoryEstimate.ByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  TLine *IniDocumentModel::allocateLine(const std::uint8_t *contents, std::size_t length) {
    static_assert(std::is_base_of<Line, TLine>::value && u8"TLine inherits from Line");

    // Figure out how much space the structure would occupy in memory, including any padding
    // before the next array element. This will let us safely created a single-allocation
    // line instance that includes the actual line contents just behind the structure.
    std::size_t alignedFooterOffset;
    {
      const TLine *dummy = nullptr;
      alignedFooterOffset = (
        reinterpret_cast<const std::uint8_t *>(dummy + 1) -
        reinterpret_cast<const std::uint8_t *>(dummy)
      );
    }

    // Now allocate the memory block and place the line structure within it
    TLine *newLine;
    {
      std::unique_ptr<std::uint8_t[]> memory(new std::uint8_t[alignedFooterOffset + length]);

      newLine = reinterpret_cast<TLine *>(memory.get());
      newLine->Contents = memory.get() + alignedFooterOffset;
      newLine->Length = length;

      std::copy_n(contents, length, newLine->Contents);

      this->createdLinesMemory.insert(memory.get());

      memory.release(); // Disown the unique_ptr so the line's memory is not freed
    }

    return newLine;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  void IniDocumentModel::freeLine(TLine *line) {
    static_assert(std::is_base_of<Line, TLine>::value && u8"TLine inherits from Line");

    std::uint8_t *memory = reinterpret_cast<std::uint8_t *>(line);
    this->createdLinesMemory.erase(memory);
    delete[] memory;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
