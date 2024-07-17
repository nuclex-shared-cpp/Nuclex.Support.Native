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

#ifndef NUCLEX_SUPPORT_TEXT_COMMANDLINE_H
#define NUCLEX_SUPPORT_TEXT_COMMANDLINE_H

#include "Nuclex/Support/Config.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores a command line parameter string in a code-friendly format</summary>
  /// <remarks>
  ///   This class can be used to prepare command lines when executing external programs
  ///   and to parse the command line passed to the running program by the operating system.
  ///   The command line is a simple parameter collection that can either be processed directly
  ///   or be used to build more complex command line binding systems on top of it.
  /// </remarks>
  class CommandLine {

    // Wraps an individual argument that has been passed on the command line
    class Argument;

    /// <summary>Initializes a new, empty command line</summary>
    public: NUCLEX_SUPPORT_API CommandLine();
    /// <summary>Releases all resources owned by the command line</summary>
    public: NUCLEX_SUPPORT_API ~CommandLine();

    /// <summary>Parses a the command line parameter from a string</summary>
    /// <param name="parameterString">String containing the command line parameters</param>
    public: NUCLEX_SUPPORT_API CommandLine Parse(const std::string &parameterString);

    //private: struct

    private: std::string parameterString;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#include "Nuclex/Support/Text/CommandLineArgument.h"

#endif // NUCLEX_SUPPORT_TEXT_COMMANDLINE_H
