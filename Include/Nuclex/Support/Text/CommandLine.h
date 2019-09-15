#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_TEXT_COMMANDLINE_H
#define NUCLEX_SUPPORT_TEXT_COMMANDLINE_H

#include "../Config.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores a command line parameter string in a code-friendly format</summary>
  /// <remarks>
  ///   This class can be used to prepare command lines when executing external programs
  ///   and to parse the command line passed to the running program by the operating system.
  ///   The command line is a simple parameter collection that can either be processed directly
  ///   or be used to build more complex command line binding systems on top of.
  /// </remarks>
  class CommandLine {

    /// <summary>Initializes a new, empty command line</summary>
    public: NUCLEX_SUPPORT_API CommandLine();
    /// <summary>Releases all resources owned by the command line</summary>
    public: NUCLEX_SUPPORT_API ~CommandLine();

    /// <summary>Parses a the command line parameter from a string</summary>
    /// <param name="parameterString">String containing the command line parameters</param>
    public: NUCLEX_SUPPORT_API CommandLine Parse(const std::string &parameterString);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_COMMANDLINE_H
