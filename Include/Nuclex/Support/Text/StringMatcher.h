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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
#define NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::u8string
#include <functional> //for std::hash, std::equal_to, std::less
#include <cstdint> // for std::uint32_t, std::uint64_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compares strings using different matching algorithms</summary>
  class NUCLEX_SUPPORT_TYPE StringMatcher {

    /// <summary>Compares two UTF-8 strings for equality, optionally ignoring case</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="left">String that will be compared on the left side</param>
    /// <param name="right">String that will be compared on the right side</param>
    /// <returns>True if the two strings are equal, false otherwise</returns>
    /// <remarks>
    ///   This method is ideal for one-off comparisons. If you have to compare one string
    ///   against multiple strings or want to create a case-insensitive string map,
    ///   consider using the <see cref="StringConverter.ToFoldedLowercase" /> method.
    /// </remarks>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static bool AreEqual(
      const std::u8string &left, const std::u8string &right
    );

    /// <summary>Checks whether one UTF-8 string contains another UTF-8 string</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="haystack">
    ///   String that will be scanned for instances of another string
    /// </param>
    /// <param name="needle">String which might appear inside the other string</param>
    /// <returns>
    ///   True if the 'needle' string appears at least once in the 'haystack' string
    /// </returns>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static bool Contains(
      const std::u8string &haystack, const std::u8string &needle
    );

    /// <summary>Finds the first occurrence of a string within another string</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="haystack">
    ///   String that will be scanned for instances of another string
    /// </param>
    /// <param name="needle">String which might appear inside the other string</param>
    /// <returns>
    ///   The index of the first 'needle' string within the 'haystack' string or
    ///   std::u8string::npos if the 'needle' was not found.
    /// </returns>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static std::u8string::size_type Find(
      const std::u8string &haystack, const std::u8string &needle
    );

    /// <summary>Checks whether one UTF-8 string starts with another UTF-8 string</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="text">
    ///   String whose beginning will be compared with the searched-for string
    /// </param>
    /// <param name="beginning">String with which the checked string must begin</param>
    /// <returns>
    ///   True if the 'haystack' string starts with the 'needle' string
    /// </returns>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static bool StartsWith(
      const std::u8string &text, const std::u8string &beginning
    );

    /// <summary>Checks whether one UTF-8 string ends with another UTF-8 string</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="text">
    ///   String whose ending will be compared with the searched-for string
    /// </param>
    /// <param name="ending">String with which the checked string must end</param>
    /// <returns>
    ///   True if the 'haystack' string ends with the 'needle' string
    /// </returns>
    /// <remarks>
    ///   This method uses some of the special properties of UTF-8 to avoid having to
    ///   scan the entire 'haystack' string for the code point at which to begin
    ///   comparing. If you feed it ANSI codepage strings with characters in the range
    ///   of 0x80 - 0xbf (128 - 191), it will give wrong results.
    /// </remarks>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static bool EndsWith(
      const std::u8string &text, const std::u8string &ending
    );

    /// <summary>Checks whether a UTF-8 string matches a wildcard</summary>
    /// <typeparam name="CaseSensitive">
    ///   Whether the comparison will be case sensitive
    /// </typeparam>
    /// <param name="text">Text that will be matched against the wildcard</param>
    /// <param name="wildcard">Wildcard against which the text will be matched</param>
    /// <returns>True if the specified text matches the wildcard</returns>
    /// <remarks>
    ///   Wildcards refer to the simple placeholder symbols employed by many shells,
    ///   where a '?' acts as a stand-in for one UTF-8 character and a '*' acts as
    ///   a stand-in for zero or more UTF-8 characters. For example &quot;*l?o*&quot;
    ///   would match &quot;Hello&quot; and &quot;lion&quot; but not &quot;glow&quot;.
    /// </remarks>
    public: template<bool CaseSensitive = false>
    NUCLEX_SUPPORT_API static bool FitsWildcard(
      const std::u8string &text, const std::u8string &wildcard
    );

  };

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::AreEqual<false>(
    const std::u8string &left, const std::u8string &right
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::AreEqual<true>(
    const std::u8string &left, const std::u8string &right
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::Contains<false>(
    const std::u8string &haystack, const std::u8string &needle
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::Contains<true>(
    const std::u8string &haystack, const std::u8string &needle
  );

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string::size_type NUCLEX_SUPPORT_API StringMatcher::Find<false>(
    const std::u8string &haystack, const std::u8string &needle
  );

  // ------------------------------------------------------------------------------------------- //

  template<> std::u8string::size_type NUCLEX_SUPPORT_API StringMatcher::Find<true>(
    const std::u8string &haystack, const std::u8string &needle
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::StartsWith<false>(
    const std::u8string &text, const std::u8string &beginning
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::StartsWith<true>(
    const std::u8string &text, const std::u8string &beginning
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::EndsWith<false>(
    const std::u8string &text, const std::u8string &ending
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::EndsWith<true>(
    const std::u8string &text, const std::u8string &ending
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::FitsWildcard<false>(
    const std::u8string &text, const std::u8string &wildcard
  );

  // ------------------------------------------------------------------------------------------- //

  template<> bool NUCLEX_SUPPORT_API StringMatcher::FitsWildcard<true>(
    const std::u8string &text, const std::u8string &wildcard
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::hash&lt;std::u8string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::unordered_map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8Hash {

    /// <summary>Calculates a case-insensitive hash of an UTF-8 string</summary>
    /// <param name="text">UTF-8 string of which a hash value will be calculated</param>
    /// <returns>The case-insensitive hash value of the provided string</returns>
    public: NUCLEX_SUPPORT_API std::size_t operator()(
      const std::u8string &text
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::equal_to&lt;std::u8string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::unordered_map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8EqualTo {

    /// <summary>Checks if two UTF-8 strings are equal, ignoring case</summary>
    /// <param name="left">First UTF-8 string to compare</param>
    /// <param name="right">Other UTF-8 string to compare</param>
    /// <returns>True if both UTF-8 strings have equal contents</returns>
    public: NUCLEX_SUPPORT_API bool operator()(
      const std::u8string &left, const std::u8string &right
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::less&lt;std::u8string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8Less {

    /// <summary>Checks if the first UTF-8 string is 'less' than the second</summary>
    /// <param name="left">First UTF-8 string to compare</param>
    /// <param name="right">Other UTF-8 string to compare</param>
    /// <returns>True if the first UTF-8 string is 'less', ignoring case</returns>
    public: NUCLEX_SUPPORT_API bool operator()(
      const std::u8string &left, const std::u8string &right
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
