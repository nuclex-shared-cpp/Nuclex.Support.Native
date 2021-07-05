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

  /// <summary>Builds the document model according to the parsed file contents</summary>
  class IniDocumentModel::ModelLoader {

    /// <summary>Amount of memory the model builder will allocate 
    public: const static std::size_t ChunkSize = 4096; // bytes

    /// <summary>Initializes a new model builder filling the specified document model</summary>
    /// <param name="targetDocumentModel">Document model the builder will fill</param>
    /// <param name="allocatedByteCount">
    ///   Number of bytes the document model has allocated as the initial chunk
    /// </param>
    public: ModelLoader(IniDocumentModel *targetDocumentModel) :
      targetDocumentModel(targetDocumentModel),
      memoryChunk(nullptr),
      remainingByteCount(0),
      currentLine(0),
      currentSection(0),
      lineStartPosition(nullptr),
      sectionStartPosition(nullptr),
      sectionEndPosition(nullptr),
      equalsSignPosition(nullptr),
      nameStartPosition(nullptr),
      nameEndPosition(nullptr),
      isMalformedLine(false) {}

    /// <summary>Notifies the model builder that a new line has begun</summary>
    /// <param name="filePosition">Current position in the parsed file</param>
    public: void BeginLine(const std::uint8_t *filePosition) {
      if(this->lineStartPosition != nullptr) {
        EndLine(filePosition);
      }

      this->lineStartPosition = filePosition;
    }

    /// <summary>Notifies the memory estimator that the current line has ended</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void EndLine(const std::uint8_t *filePosition) {
      if(!this->isMalformedLine) {
        if(this->equalsSignPosition != nullptr) { // Is it a property assignment line?
          PropertyLine *newLine = addLine<PropertyLine>(this->lineStartPosition, filePosition);
        } else if(this->sectionEndPosition != nullptr) { // end is only set when start is set
          SectionLine *newLine = addLine<SectionLine>(this->lineStartPosition, filePosition);
          /*
          newLine->NameStartIndex = this->nameStartPosition;
          if(this->nameEndPosition == nullptr) {
            newLine->NameLength = 0;
          } else {
            newLine->NameLength = (this->nameEndPosition - this->nameStartPosition) + 1;
          }
          */
        } else { // It's a meaningless line
          Line *newLine = addLine<Line>(this->lineStartPosition, filePosition);
        }
      }

      this->lineStartPosition = filePosition;
      
      this->sectionStartPosition = nullptr;
      this->sectionEndPosition = nullptr;
      this->equalsSignPosition = nullptr;
      this->isMalformedLine = false;
    }

    /// <summary>Notifies the memory estimator that a section has been opened</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void BeginSection(const std::uint8_t *filePosition) {
      if(this->sectionEndPosition != nullptr) {

        // If there is a previous section, that means there are two sections in the same
        // line and we need to cut the current line in two
        if(this->sectionEndPosition == nullptr) {
          this->isMalformedLine = true;
        } else {
          EndLine(filePosition);
        }

      }

      this->sectionStartPosition = filePosition;
    }

    /// <summary>Notifies the memory estimator that a section has been closed</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    /// <param name="nameStart">Position in the file where the section name begins</param>
    /// <param name="nameEnd">File position one after the end of the section name</param>
    public: void EndSection(const std::uint8_t *filePosition) {
      if(this->sectionStartPosition == nullptr) {
        this->isMalformedLine = true;
      } else if(this->sectionEndPosition == nullptr) {
        this->sectionEndPosition = filePosition;
      } else {
        this->isMalformedLine = true;
      }
    }

    /// <summary>Notifies the memory estimator that an equals sign has been found</summary>
    /// <param name="filePosition">Current position in the file scan</param>
    public: void AddAssignment(const std::uint8_t *filePosition) {
      if(this->equalsSignPosition == nullptr) {
        this->equalsSignPosition = filePosition;
      } else {
        this->isMalformedLine = true;
      }
    }

    /// <summary>Sets position of the element name encountered while scanning</summary>
    /// <param name="nameStartPosition">Position at which the element name begins</param>
    /// <param name="nameEndPosition">Position one after the element name ends</param>
    public: void SetName(
      const std::uint8_t *nameStartPosition, const std::uint8_t *nameEndPosition
    ) {
      this->nameStartPosition = nameStartPosition;
      this->nameEndPosition = nameEndPosition;
    }

    private: template<typename TLine>
    TLine *addLine(const std::uint8_t *lineBegin, const std::uint8_t *lineEnd) { 
      TLine *newLine = this->targetDocumentModel->allocateLine<TLine>(
        lineBegin, (lineEnd - lineBegin) + 1
      );
      if(this->currentLine == nullptr) {
        newLine->Previous = newLine;
        newLine->Next = newLine;
      } else {
        newLine->Previous = this->currentLine;
        newLine->Next = this->currentLine->Next;

        newLine->Next->Previous = newLine;
        newLine->Previous->Next = newLine;
      }

      this->currentLine = newLine;

      return newLine;
    }

    /// <summary>Document model this builder will fill with parsed elements</summary>
    private: IniDocumentModel *targetDocumentModel;
    /// <summary>Memory chunk into which parsed lines are constructed</summary>
    private: std::uint8_t *memoryChunk;
    /// <summary>Remaining bytes available of the current memory chunk</summary>
    private: std::size_t remainingByteCount;
    private: Line *currentLine;
    private: IndexedSection *currentSection;

    /// <summary>File offset at which the current line begins</summary>
    private: const std::uint8_t *lineStartPosition;
    /// <summary>File offset at which the current section starts, if any</summary>
    private: const std::uint8_t *sectionStartPosition;
    /// <summary>File offset one after the end of the current section, if any</summary>
    private: const std::uint8_t *sectionEndPosition;
    /// <summary>File offset at which the current assignment's equals sign is</summary>
    private: const std::uint8_t *equalsSignPosition;

    /// <summary>File offset at which the current section/property's name starts</summary>
    private: const std::uint8_t *nameStartPosition;
    /// <summary>File offset one after the end of the current section/property's name</summary>
    private: const std::uint8_t *nameEndPosition;

    /// <summary>Do we have conclusive evidence that the line is malformed?</summary>
    private: bool isMalformedLine;

  };

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
    ModelLoader modelLoader(this);
    {
      bool previousWasNewLine = false;
      bool isInsideQuote = false;
      bool encounteredComment = false;
      //bool nameComplete = false;

      // Make sure the memory estimator knows where the first line starts
      modelLoader.BeginLine(fileContents);

      const std::uint8_t *nameBegin = nullptr;
      const std::uint8_t *nameEnd = nullptr;

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
          nameBegin = nameEnd = nullptr;
          modelLoader.EndLine(fileBegin + 1);
        } else if(isInsideQuote) { // Quotes make it ignore section and assignment characters
          if(current == '"') {
            isInsideQuote = false;
          }
        } else if(!encounteredComment) { // Otherwise, parse until comment encountered
          switch(current) {
            case ';':
            case '#': { encounteredComment = true; break; }
            case '"': { isInsideQuote = true; break; }
            case '[': {
              nameBegin = nameEnd = nullptr;
              modelLoader.BeginSection(fileBegin);
              break;
            }
            case ']': {
              modelLoader.SetName(nameBegin, nameEnd);
              modelLoader.EndSection(fileBegin);
              break;
            }
            case '=': {
              modelLoader.SetName(nameBegin, nameEnd);
              modelLoader.AddAssignment(fileBegin);
              break;
            }
            default: { break; }
          }
        }

        // Keep track of where the section or property name starts & ends
        if(!isWhitepace(current) && (current != '[')) {
          if(nameBegin == nullptr) {
            nameBegin = fileBegin;
          } else {
            nameEnd = fileBegin;
          } 
        }

        ++fileBegin;
      } // while fileBegin < fileEnd

      // If the file didn't end with a line break, consider the contents of
      // the file's final line as another line
      if(!previousWasNewLine) {
        modelLoader.EndLine(fileEnd);
      }
    } // beauty scope
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  TLine *IniDocumentModel::allocateLine(const std::uint8_t *contents, std::size_t length) {
    static_assert(std::is_base_of<Line, TLine>::value && u8"TLine inherits from Line");

    // Allocate memory for a new line and assign its content pointer to hold
    // the line loaded from the .ini file.
    TLine *newLine = allocate<TLine>(length);
    newLine->Contents = (
      reinterpret_cast<std::uint8_t *>(newLine) + getSizePlusAlignmentPadding<TLine>()
    );
    newLine->Length = length;

    std::copy_n(contents, length, newLine->Contents);

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

  template<typename T>
  T *IniDocumentModel::allocate(std::size_t extraByteCount /* = 0 */) {

    // While we're asked to allocate a specific type, making extra bytes available
    // requires us to allocate as std::uint8_t. The start address still needs to be
    // appropriately aligned for the requested type (otherwise we'd have to keep
    // separate pointers for delete[] and for the allocated type).
    #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(Line));
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(SectionLine));
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(PropertyLine));
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(IndexedSection));
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
