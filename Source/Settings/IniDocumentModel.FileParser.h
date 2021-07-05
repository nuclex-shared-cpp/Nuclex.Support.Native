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

    /// <summary>Sets the document model that will be filled by the parser</summary>
    /// <param name="documentModel">
    ///   Document model into which the parsed properties will be written
    /// </param>
    public: void SetDocumentModel(IniDocumentModel *documentModel);

    /// <summary>Parses the .ini file and fills the specified document model</summary>
    public: void Parse();

    /// <summary>The document model into this parser will fill</summary>
    private: IniDocumentModel *target;

    /// <summary>Pointer to the beginning of the .ini file in memory</summary>
    private: const std::uint8_t *fileBegin;
    /// <summary>Pointer one past the end of the .ini file in memory</memory>
    private: const std::uint8_t *fileEnd;


  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
