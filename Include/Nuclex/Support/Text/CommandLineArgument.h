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
