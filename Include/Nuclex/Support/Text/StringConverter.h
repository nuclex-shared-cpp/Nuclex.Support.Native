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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
#define NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <filesystem> // for std::filesystem::path

namespace Nuclex::Support::Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts strings between explicitly specified UTF-formats</summary>
  /// <remarks>
  ///   <para>
  ///     UTF-8 is the de-facto standard of the web and on all mobile and desktop platforms
  ///     in general. That's because it is compact, has no endian issues, can resync to
  ///     the very next letter when data gets corrupted and is overall well defined.
  ///     However, there's an operating system where many things went very wrong...
  ///   </para>
  ///   <para>
  ///     On Windows, 8 bit char strings are usually assumed to be ANSI (that is,
  ///     the 127 standard ASCII characters for the values 1-127 and a set of special
  ///     glyphs that are defined by the &quot;code page&quot; in the remaining values
  ///     from 128 to 255. Showing strings with the wrong code page displays the wrong
  ///     special characters (but ASCII characters remain intact).
  ///   </para>
  ///   <para>
  ///     On Windows, wchar_t is 16 bits wide and unicode generally means UTF-16,
  ///     so &quot;wide strings&quot; (strings holding wchar_ts) are UTF-16 strings.
  ///     Microsoft's unicode APIs and UI tooling uses this for all i18n support.
  ///   </para>
  ///   <para>
  ///     On platforms other than Windows, wchar_t is instead 32 bits wide and compilers
  ///     like GCC default to using UTF-32 when seeing a string like L&quot;hello&quot;.
  ///     You can force GCC to put UTF-16 in the 32 bit wide wchar_ts (via
  ///     -fwide-exec-charset=UTF-16) but then every other library accepting
  ///     &quot;wide strings&quot; will be confused by your UTF-16-inside-UTF-32-strings.
  ///   </para>
  ///   <para>
  ///     So, to summarize: u8&quot;Hello&quot; works everywhere. L&quot;Hello&quot; works
  ///     for calls inside Windows applications to Microsoft APIs but gives you a headache
  ///     in any other case. This class provides a few wrappers to convert between strings
  ///     using the different UTF encodings. When interacting with Microsoft APIs in Windows
  ///     builds, translate via <see cref="WideFromUtf8" /> and <see cref="Utf8FromWide" />.
  ///     If you need to transmit UTF-16 over the network to Microsoft systems, use
  ///     <see cref="Utf16FromUtf8" /> and <see cref="Utf8FromUtf16" /> to portably
  ///     translate to and from UTF-16 encoded strings.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE StringConverter {

    /// <summary>Counts the number of UTF-8 letters in a string</summary>
    /// <param name="from">UTF-8 string whose letters will be counted</param>
    /// <returns>The number of UTF-8 letters the string is holding</returns>
    public: NUCLEX_SUPPORT_API static std::u8string::size_type CountUtf8CodePoints(
      const std::u8string &from
    );

    /// <summary>Converts a UTF-8 string into a wide (UTF-16 or UTF-32) string</summary>
    /// <param name="from">UTF-8 string that will be converted</param>
    /// <returns>A wide version of the provided UTF-8 string</returns>
    /// <remarks>
    ///   Assumes std::wstring has to carry either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   and the defaults of any wide-character APIs on your platform.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::wstring WideFromUtf8(const std::u8string &from);

    /// <summary>Converts a wide (UTF-16 or UTF-32) string into a UTF-8 string</summary>
    /// <param name="from">Wide string that will be converted</param>
    /// <returns>A UTF-8 version of the provided wide string</returns>
    /// <remarks>
    ///   Assumes the std::wstring is carrying either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   when you write L&quot;Hello&quot; in your code.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::u8string Utf8FromWide(const std::wstring &from);

    /// <summary>Converts a UTF-8 string into a 'char'-based string that's still UTF-8</summary>
    /// <param name="from">UTF-8 string that will be converted</param>
    /// <returns>A 'char'-based version of the provided UTF-8 string</returns>
    /// <remarks>
    ///   Historically, 'char' was used for ASCII text, but the UNIX and Linux world early on
    ///   did a clean switch to UTF-8, so 'char' is synonymous with UTF-8 character (of which
    ///   1 to 4 form a code point, aka a letter), while Windows did the whole regional ANSI
    ///   codepage mess, thus, overall, `std::string` might contain ASCII, ANSI or UTF-8.
    ///   This method just mushes the UTF-8 characters into an std::string unchanged.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::string CharFromUtf8(const std::u8string &from);

    /// <summary>Converts a 'char'-based (but UTF-8 encoded) string into a UTF-8 string</summary>
    /// <param name="from">'char'-based string holding UTF-8 text that will be converted</param>
    /// <returns>A UTF-8 version of the provided 'char'-based string</returns>
    /// <remarks>
    ///   This assumes that the provided string already contains UTF-8 characters that just
    ///   so happen to be stored in a 'char'-based string for compatibility reasons.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::u8string Utf8FromChar(const std::string &from);

    /// <summary>Converts a UTF-8 string into a UTF-16 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A UTF-16 version of the provided UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API static std::u16string Utf16FromUtf8(
      const std::u8string &utf8String
    );

    /// <summary>Converts a UTF-16 string into a UTF-8 string</summary>
    /// <param name="utf16String">UTF-16 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-16 string</returns>
    public: NUCLEX_SUPPORT_API static std::u8string Utf8FromUtf16(
      const std::u16string &utf16String
    );

    /// <summary>Converts a UTF-8 string into a UTF-32 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A UTF-32 version of the provided UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API static std::u32string Utf32FromUtf8(
      const std::u8string &utf8String
    );

    /// <summary>Converts a UTF-32 string into a UTF-8 string</summary>
    /// <param name="utf32String">UTF-32 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-32 string</returns>
    public: NUCLEX_SUPPORT_API static std::u8string Utf8FromUtf32(
      const std::u32string &utf32String
    );

    /// <summary>Appends a path as UTF-8 characters to a UTF-8 string</summary>
    /// <param name="target">UTF-8 string to which a string will be appended</param>
    /// <param name="pathToAppend">Path that will be appended</param>
    public: NUCLEX_SUPPORT_API static void AppendPathAsUtf8(
      std::u8string &target, const std::filesystem::path &pathToAppend
    );

    /// <summary>Appends a path as wide characters to a wide string</summary>
    /// <param name="target">Wide string to which the path will be appended</param>
    /// <param name="pathToAppend">Path that will be appended</param>
    public: NUCLEX_SUPPORT_API static void AppendPathAsWide(
      std::wstring &target, const std::filesystem::path &pathToAppend
    );

    /// <summary>Converts the specified UTF-8 string to &quot;folded lowercase&quot;</summary>
    /// <param name="utf8String">String that will be converted</param>
    /// <returns>An equivalent-ish string using only lowercase characters</returns>
    /// <remarks>
    ///   Folded lowercase is a special variant of lowercase that will result in a string of
    ///   equal or shorter length (codepoint-wise). It is not guaranteed to always give the
    ///   correct result for a human reading the string (though in the vast majority of cases
    ///   it does) -- its purpose is to enable case-insensitive comparison of strings.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::u8string FoldedLowercaseFromUtf8(
      const std::u8string &utf8String
    );

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
