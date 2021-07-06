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

#ifndef NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
#define NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H

#include "Nuclex/Support/Config.h"

#include "IniDocumentModel.h"

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Builds a document model by parses an existing .ini file</summary>
  class IniDocumentModel::FileParser {

    /// <summary>Initializes a new .ini file parser</summary>
    /// <param name="fileContents">Full file contents of the .ini file in memory</param>
    /// <param name="byteCount">Length of the .ini file in bytes</param>
    public: FileParser(const std::uint8_t *fileContents, std::size_t byteCount);

    /// <summary>Parses the .ini file and fills the specified document model</summary>
    /// <param name="documentModel">
    ///   Document model into which the parsed properties will be written
    /// </param>
    public: void ParseInto(IniDocumentModel *documentModel);

    /// <summary>Parses a comment, must be called on the comment start character</summary>
    private: void parseComment();

    /// <summary>Parses a property or section name, must be called on first character</summary>
    private: void parseName();

    /// <summary>Parses a property value, must be called on first character</summary>
    private: void parseValue();

    /// <summary>Parses an invalid line until the next line break</summary>
    private: void parseMalformedLine();

    /// <summary>Submits was has been parsed so far as a line</summary>
    private: void submitLine();

    //private: Line *generateMeaninglessLine();
    private: PropertyLine *generatePropertyLine();
    private: SectionLine *generateSectionLine();

    /// <summary>Resets the parser state</summary>
    private: void resetState();

    /// <summary>Allocates memory for a single line</summary>
    /// <typeparam name="TLine">Type of line that will be allocated</typeparam>
    /// <param name="contents">The bytes this line consists of, including CR / CR-LF</param>
    /// <param name="length">Length of the line in bytes</param>
    /// <returns>The new line</returns>
    private: template<typename TLine>
    TLine *allocateLine(const std::uint8_t *contents, std::size_t length);

    /// <summary>The document model into this parser will fill</summary>
    private: IniDocumentModel *target;
    /// <summary>Section into which parsed elements go currently</summary>
    private: IndexedSection *currentSection;
    /// <summary>Most recent parsed line</summary>
    private: Line *currentLine;

    /// <summary>Pointer to the beginning of the .ini file in memory</summary>
    private: const std::uint8_t *fileBegin;
    /// <summary>Pointer one past the end of the .ini file in memory</memory>
    private: const std::uint8_t *fileEnd;
    /// <summary>Pointer to the current parsing location</summary>
    private: const std::uint8_t *parsePosition;

    /// <summary>Position at which the current line in the .ini file begins</summary>
    private: const std::uint8_t *lineStart;
    /// <summary>Position at which the current section or property's name starts</summary>
    private: const std::uint8_t *nameStart;
    /// <summary>Position one after the end of the current section or property name</summary>
    private: const std::uint8_t *nameEnd;
    /// <summary>Position at which the current property's value starts, if any</summary>
    private: const std::uint8_t *valueStart;
    /// <summary>Position one after the end of the current property's value, if any</summary>
    private: const std::uint8_t *valueEnd;

    /// <summary>Whether a section was found in the current line</summary>
    private: bool sectionFound;
    /// <summary>Whether an equals sign was found in the current line</summary>
    private: bool equalsSignFound;
    /// <summary>Whether we encountered something that breaks the current line</summary>
    private: bool lineIsMalformed;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
