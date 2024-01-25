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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGHELPER_H
#define NUCLEX_SUPPORT_TEXT_STRINGHELPER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides a few additional helper methods for dealing with strings</summary>
  class NUCLEX_SUPPORT_TYPE StringHelper {

    /// <summary>Removes any whitespace characters that follow other whitespace</summary>
    /// <param name="utf8String">String in which duplicate whitespace will be collapsed</param>
    /// <param name="alsoTrim">Whether to also remove leading and trailing whitespace</param>
    public: NUCLEX_SUPPORT_API static void CollapseDuplicateWhitespace(
      std::string &utf8String, bool alsoTrim = true
    );

    /// <summary>Removes any whitespace characters that follow other whitespace</summary>
    /// <param name="utf8String">String in which duplicate whitespace will be collapsed</param>
    /// <param name="alsoTrim">Whether to also remove leading and trailing whitespace</param>
    public: NUCLEX_SUPPORT_API static void CollapseDuplicateWhitespace(
      std::wstring &wideString, bool alsoTrim = true
    );

    /// <summary>Removes all occurrences of a substring from the master string</summary>
    /// <param name="master">
    ///   String from which all occurrences of the specified substring will be removed
    /// </param>
    /// <param name="substringToRemove">
    ///   Substring that will be removed from the master string
    /// </param>
    /// <remarks>
    ///   This method is guaranteed to not remove occurrences recursively. For example,
    ///   removing "<startend>" from the string "Test<start<startend>end>" will produce
    ///   the string "Test<startend>" (i.e. it will only remove the substring where it
    ///   was present initially, not where it was formed as an effect of the removal).
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void EraseSubstrings(
      std::string &utf8String, const std::string &victim
    );

    /// <summary>Removes all occurrences of a substring from the master string</summary>
    /// <param name="master">
    ///   String from which all occurrences of the specified substring will be removed
    /// </param>
    /// <param name="substringToRemove">
    ///   Substring that will be removed from the master string
    /// </param>
    /// <remarks>
    ///   This method is guaranteed to not remove occurrences recursively. For example,
    ///   removing "<startend>" from the string "Test<start<startend>end>" will produce
    ///   the string "Test<startend>" (i.e. it will only remove the substring where it
    ///   was present initially, not where it was formed as an effect of the removal).
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void EraseSubstrings(
      std::wstring &wideString, const std::wstring &victim
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGHELPER_H
