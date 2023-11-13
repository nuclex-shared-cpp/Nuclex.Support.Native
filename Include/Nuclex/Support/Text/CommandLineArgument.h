#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_TEXT_COMMANDLINEARGUMENT_H
#define NUCLEX_SUPPORT_TEXT_COMMANDLINEARGUMENT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/CommandLine.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Single argument in a command line parameter string</summary>
  class CommandLine::Argument {

    /// <summary>Initializes a new, empty command line</summary>
    public: NUCLEX_SUPPORT_API Argument();
    /// <summary>Releases all resources owned by the command line</summary>
    public: NUCLEX_SUPPORT_API ~Argument();

    /// <summary>Provides the whole argument as it was specified on the command line</summary>
    /// <returns>The command line argument without any modifications</returns>
    public: NUCLEX_SUPPORT_API std::string GetRaw() const;

    /// <summary>Retrieves the initiator characters of this argument</summary>
    /// <returns>The characters used to initiate the argument</returns>
    /// <remarks>
    ///   <para>
    ///     The initiator is a special character used to denote the start of a command line
    ///     argument, typically either '-', '--' (Unix) or '/' (Windows). Loose arguments
    ///     (typically used for commands or to specify file names) do not have initiators.
    ///   </para>
    ///   <para>
    ///
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual std::string GetInitiator() const = 0;

    /// <summary>Retrieves the name of the argument</summary>
    /// <returns>The name of the command line argument</returns>
    /// <remarks>
    ///   Name of the argument, if it was passed as an option. Loose arguments (such as
    ///   a filename or simple command) do not have names.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual std::string GetName() const = 0;

    /// <summary>Retrieves the associator between the argument and its value</summary>
    /// <returns>The character sequence used to associate a value with the argument</returns>
    public: NUCLEX_SUPPORT_API virtual std::string GetAssociator() const = 0;

    /// <summary>Retrieves the value of the argument</summary>
    /// <returns>The value specified on the command line</returns>
    public: NUCLEX_SUPPORT_API virtual std::string GetValue() const = 0;

    private: std::string::size_type initiatorStartIndex;
    private: std::string::size_type initiatorLength;
    private: std::string::size_type nameStartIndex;
    private: std::string::size_type nameLength;
    private: std::string::size_type associatorStartIndex;
    private: std::string::size_type associatorLength;
    private: std::string::size_type valueStartIndex;
    private: std::string::size_type valueLength;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_COMMANDLINEARGUMENT_H
