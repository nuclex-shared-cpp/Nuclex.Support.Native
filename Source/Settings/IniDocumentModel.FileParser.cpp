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
//   Foo = Bar = Baz -> Malformed
//   [Yay = Nay]     -> Malformed
//   "Hello          -> Malformed
//   Foo = [Bar]     -> Assignment, no section
//   Foo = ]][Bar    -> Assignment
//   "Foo" Bar = Baz -> Malformed
//   Foo = "Bar" Baz -> Malformed
//

// Considered allocation schemes:
//
//   By line                      -> lots of micro-allocations
//   In blocks (custom allocator) -> I have to do reference counting to free anything
//   Load pre-alloc, then by line -> Fast for typical case, no or few micro-allocations
//                                   But requires pre-scan of entire file + more code

// This could be done with tried-and-proven parser generators such as class Flex/Yacc/Bison
// or Boost.Spirit. However, I wanted something lean, fast and without external dependencies.
//
// A middleground option would be a modern PEG parser generator like this:
//   https://github.com/TheLartians/PEGParser
//
// But the implementation below, even if tedious, gets the job done, fast and efficient.
//

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
    fileEnd(fileContents + byteCount),
    parsePosition(nullptr),
    lineStart(nullptr),
    nameStart(nullptr),
    nameEnd(nullptr),
    valueStart(nullptr),
    valueEnd(nullptr),
    sectionFound(false),
    equalsSignFound(false),
    lineIsMalformed(false) {}

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::ParseInto(IniDocumentModel *documentModel) {
    this->target = documentModel;

    // Reset the parser, just in case someone re-uses an instance
    resetState();
    this->currentSection = nullptr;
    this->currentLine = nullptr;

    // Go through the entire file contents byte-by-byte and select the correct parse
    // mode for the elements we encounter. All of these characters are in the ASCII range,
    // thus there are no UTF-8 sequences that could be mistaken for them (multi-byte UTF-8
    // codepoints will have the highest bit set in all bytes)
    this->parsePosition = this->lineStart = this->fileBegin;
    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      switch(current) {

        // Comments (any section or property already found still counts)
        case '#':
        case ';': { parseComment(); break; }

        // Equals sign, line is a property assignment
        case '=': {
          if(equalsSignFound) {
            parseMalformedLine();
          } else {
            this->equalsSignFound = true;
            ++this->parsePosition;
          }
          break;
        }

        // Line break, submits the current line to the document model
        case '\n': {
          submitLine();
          break;
        }

        // Other character, parse as section name, property name or property value
        default: {
          if(isWhitepace(current)) {
            ++this->parsePosition; // skip over it
          } else if(equalsSignFound) {
            parseValue();
          } else {
            parseName();
          }
          break;
        }

      } // switch on current byte
    } // while parse position is before end of file
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseComment() {
    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      if(current == '\n') {
        break;
      } else { // Skip everything that isn't a newline character
        ++this->parsePosition;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseName() {
    bool isInQuote = false;
    bool quoteEncountered = false;
    bool isInSection = false;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;

      // When inside a quote, ignore everything but the closing quote
      // (or newline / end-of-file which are handled in all cases)
      if(isInQuote) {
        isInQuote = (current != '"');
        nameEnd = this->parsePosition;
      } else { // Outside of quote
        switch(current) {

          // Section start found?
          case '[': {
            if((this->nameStart != nullptr) || isInSection) { // Bracket is not first char?
              parseMalformedLine();
              return;
            } else if(this->sectionFound) { // Did we already see a section in this line?
              submitLine();
            }

            isInSection = true;
            nameStart = this->parsePosition + 1;
            break;
          }

          // Section end found?
          case ']': {
            if((this->nameStart == nullptr) || !isInSection) { // Bracket is first char?
              parseMalformedLine();
              return;
            }

            isInSection = false;
            this->nameEnd = this->parsePosition;
            this->sectionFound = true;
            break;
          }

          // Quoted name found?
          case '"': {
            if((this->nameStart != nullptr) || quoteEncountered) { // Quote is not first char?
              parseMalformedLine();
              return;
            } else { // Quote is first char encountered
              quoteEncountered = true;
              isInQuote = true;
              nameStart = this->parsePosition + 1;
            }
            break;
          }

          // Equals sign found? The name part is over, assignment follows
          case '=': {
            if(isInSection) { // Equals sign inside section name? -> line is malformed
              parseMalformedLine();
            }
            //this->equalsSignFound = true;
            return;
          }

          // Other characters without special meaning
          default: {
            if(!isWhitepace(current)) {
              if(quoteEncountered) { // Characters after quote? -> line is malformed
                parseMalformedLine();
                return;
              }
              if(nameStart == nullptr) {
                nameStart = this->parsePosition;
              }
              nameEnd = this->parsePosition + 1;
            }
            break;
          }

        } // switch on current byte
      } // is outside of quote

      // When a newline character is encountered, the name ends
      if(current == '\n') {
        if(isInQuote) { // newline inside a quote? -> line is malformed
          this->lineIsMalformed = true;
        }
        return;
      }

      ++this->parsePosition;
    } // while parse position is before end of file
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseValue() {
    bool isInQuote = false;
    bool quoteEncountered = false;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;

      // When inside a quote, ignore everything but the closing quote
      // (or newline / end-of-file which are handled in all cases)
      if(isInQuote) {
        isInQuote = (current != '"');
        valueEnd = this->parsePosition;
      } else { // Outside of quote
        switch(current) {

          // Quoted value found?
          case '"': {
            if((this->valueStart != nullptr) || quoteEncountered) { // Quote is not first char?
              parseMalformedLine();
              return;
            } else { // Quote is first char encountered
              quoteEncountered = true;
              isInQuote = true;
              valueStart = this->parsePosition + 1;
            }
            break;
          }

          // Another equals sign found? -> line is malformed
          case '=': {
            parseMalformedLine();
            return;
          }

          // Other characters without special meaning
          default: {
            if(!isWhitepace(current)) {
              if(quoteEncountered) { // Characters after quote? -> line is malformed
                parseMalformedLine();
                return;
              }
              if(valueStart == nullptr) {
                valueStart = this->parsePosition;
              }
              valueEnd = this->parsePosition + 1;
            }
            break;
          }

        } // switch on current byte
      } // is outside of quote

      // When a newline character is encountered, the name ends
      if(current == '\n') {
        if(isInQuote) { // newline inside a quote? -> line is malformed
          this->lineIsMalformed = true;
        }
        return;
      }

      ++this->parsePosition;
    } // while parse position is before end of file
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::parseMalformedLine() {
    this->lineIsMalformed = true;

    while(this->parsePosition < this->fileEnd) {
      std::uint8_t current = *this->parsePosition;
      if(current == '\n') {
        break;
      }

      ++this->parsePosition;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::submitLine() {
    ++this->parsePosition;

    Line *newLine;
    if(this->lineIsMalformed) {
      /*
      newLine = this->target->allocateLine<Line>(
        this->lineStart, this->parsePosition - this->lineStart
      );
      */
      newLine = new Line();
    } else if(this->equalsSignFound) {
      newLine = generatePropertyLine();
    } else if(this->sectionFound) {
      newLine = generateSectionLine();
    } else {
      /*
      newLine = this->target->allocateLine<Line>(
        this->lineStart, this->parsePosition - this->lineStart
      );
      */
      newLine = new Line();
    }


    resetState();
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::PropertyLine *IniDocumentModel::FileParser::generatePropertyLine() {
    /*
    PropertyLine *newPropertyLine = this->target->allocateLine<PropertyLine>(
      this->lineStart, this->parsePosition - this->lineStart
    );
    */
    PropertyLine *newPropertyLine = new PropertyLine();

    if((this->nameStart != nullptr) && (this->nameEnd != nullptr)) {
      newPropertyLine->NameStartIndex = this->nameStart - this->lineStart;
      newPropertyLine->NameLength = this->nameEnd - this->nameStart;
    } else {
      newPropertyLine->NameStartIndex = 0;
      newPropertyLine->NameLength = 0;
    }

    if((this->valueStart != nullptr) && (this->valueEnd != nullptr)) {
      newPropertyLine->ValueStartIndex = this->valueStart - this->lineStart;
      newPropertyLine->ValueLength = this->valueEnd - this->nameStart;
    } else {
      newPropertyLine->ValueStartIndex = 0;
      newPropertyLine->ValueLength = 0;
    }

    return newPropertyLine;
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::SectionLine *IniDocumentModel::FileParser::generateSectionLine() {
    /*
    SectionLine *newSectionLine = this->target->allocateLine<SectionLine>(
      this->lineStart, this->parsePosition - this->lineStart
    );
    */
    SectionLine *newSectionLine = new SectionLine(); // TODO: Use chunk allocator

    if((this->nameStart != nullptr) && (this->nameEnd != nullptr)) {
      newSectionLine->NameStartIndex = this->nameStart - this->lineStart;
      newSectionLine->NameLength = this->nameEnd - this->nameStart;
    } else {
      newSectionLine->NameStartIndex = 0;
      newSectionLine->NameLength = 0;
    }

    return newSectionLine;
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::FileParser::resetState() {
    this->lineStart = this->parsePosition;

    this->nameStart = this->nameEnd = nullptr;
    this->valueStart = this->valueEnd = nullptr;

    this->sectionFound = this->equalsSignFound = this->lineIsMalformed = false;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
