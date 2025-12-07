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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGHELPER_H
#define NUCLEX_SUPPORT_TEXT_STRINGHELPER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::u8string
#include <string_view> // for std::u8string_view

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides a few additional helper methods for dealing with strings</summary>
  class NUCLEX_SUPPORT_TYPE StringHelper {

    /// <summary>Removes any whitespace characters that follow other whitespace</summary>
    /// <param name="utf8String">String in which duplicate whitespace will be collapsed</param>
    /// <param name="alsoTrim">Whether to also remove leading and trailing whitespace</param>
    /// <remarks>
    ///   This method considers all whitespace characters defined by unicode. It will leave
    ///   single whitespace characters intact, but consecutive whitespace characters will
    ///   be replaced with a single ascii whitespace. A string consisting of only whitspace
    ///   will result either in a single whitespace remaining or nothing if trim is enabled.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void CollapseDuplicateWhitespace(
      std::u8string &utf8String, bool alsoTrim = true
    );

    /// <summary>Removes any whitespace characters that follow other whitespace</summary>
    /// <param name="wideString">String in which duplicate whitespace will be collapsed</param>
    /// <param name="alsoTrim">Whether to also remove leading and trailing whitespace</param>
    /// <remarks>
    ///   This method considers all whitespace characters defined by unicode. It will leave
    ///   single whitespace characters intact, but consecutive whitespace characters will
    ///   be replaced with a single ascii whitespace. A string consisting of only whitspace
    ///   will result either in a single whitespace remaining or nothing if trim is enabled.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void CollapseDuplicateWhitespace(
      std::wstring &wideString, bool alsoTrim = true
    );

    /// <summary>Removes all occurrences of a substring from the master string</summary>
    /// <param name="utf8String">
    ///   String from which all occurrences of the specified substring will be removed
    /// </param>
    /// <param name="victim">
    ///   Substring that will be removed from the master string
    /// </param>
    /// <remarks>
    ///   This method is guaranteed to not remove occurrences recursively. For example,
    ///   removing "<startend>" from the string "Test<start<startend>end>" will produce
    ///   the string "Test<startend>" (i.e. it will only remove the substring where it
    ///   was present initially, not where it was formed as an effect of the removal).
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void EraseSubstrings(
      std::u8string &utf8String, const std::u8string &victim
    );

    /// <summary>Removes all occurrences of a substring from the master string</summary>
    /// <param name="wideString">
    ///   String from which all occurrences of the specified substring will be removed
    /// </param>
    /// <param name="victim">
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

    /// <summary>Returns a whitespace-trimmed segment of the input string</summary>
    /// <param name="utf8String">String in UTF-8 format that will be trimmed</param>
    /// <returns>A whitespace-trimmed segment of the input string</returns>
    /// <remarks>
    ///   This is based on string_view because trimming is non-destructive and can
    ///   return a view into the original string without copying it. There is little overhead
    ///   as UTF-8 enumeration requires pointers to the string's data and length to
    ///   be taken anyway.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::u8string_view GetTrimmed(
      const std::u8string_view &utf8String
    );

    /// <summary>Returns a whitespace-trimmed segment of the input string</summary>
    /// <param name="utf8String">String in UTF-8 format that will be trimmed</param>
    /// <returns>A whitespace-trimmed segment of the input string</returns>
    /// <remarks>
    ///   This is based on string_view because trimming is non-destructive and can
    ///   return a view into the original string without copying it. There is little overhead
    ///   as UTF-8 enumeration requires pointers to the string's data and length to
    ///   be taken anyway.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::wstring_view GetTrimmed(
      const std::wstring_view &wideString
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGHELPER_H
