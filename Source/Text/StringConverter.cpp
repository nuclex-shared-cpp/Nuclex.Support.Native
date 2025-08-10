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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/Text/UnicodeHelper.h" // UTF encoding and decoding

#include <stdexcept> // for std::invalid_argument

namespace {

  // ------------------------------------------------------------------------------------------- //



  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::u8string::size_type StringConverter::CountUtf8CodePoints(const std::u8string &from) {
    const char8_t *current = from.c_str();
    const char8_t *end = current + from.length();

    std::u8string::size_type count = 0;
    while(current < end) {
      std::size_t sequenceLength = UnicodeHelper::GetSequenceLength(*current);
      if(sequenceLength == std::size_t(-1)) {
        throw std::invalid_argument(
          reinterpret_cast<const char *>(u8"String contains invalid UTF-8")
        );
      }

      ++count;
      current += sequenceLength;
    }

    return count;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::CharFromUtf8(const std::u8string &utf8String) {
    return std::string(utf8String.begin(), utf8String.end());
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string StringConverter::Utf8FromChar(const std::string &charString) {
    return std::u8string(charString.begin(), charString.end());
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::WideFromUtf8(const std::u8string &utf8String) {
    std::wstring result;
    {
      const char8_t *read = utf8String.c_str();
      const char8_t *readEnd = read + utf8String.length();

      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      // Variant for 16 bit wchar_t as established by Windows compilers
      if constexpr(sizeof(wchar_t) == sizeof(char16_t)) {
        char16_t *write = reinterpret_cast<char16_t *>(result.data());

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          UnicodeHelper::WriteCodePoint(write, codePoint);
        }

        result.resize(write - reinterpret_cast<char16_t *>(result.data()));
      } else { // Variant for 32 bit wchar_t used everywhere except Windows
        char32_t *write = reinterpret_cast<char32_t *>(result.data());

        while(read < readEnd) {
          *write = UnicodeHelper::ReadCodePoint(read, readEnd);
          ++write;
        }

        result.resize(write - reinterpret_cast<char32_t *>(result.data()));
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string StringConverter::Utf8FromWide(const std::wstring &wideString) {
    std::u8string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(wideString.length() * 2 + 2);

      char8_t *write = result.data();
      char8_t *writeEnd = write + result.length();

      // Variant for 16 bit wchar_t as established by Windows compilers
      if constexpr(sizeof(wchar_t) == sizeof(char16_t)) {
        const char16_t *read = reinterpret_cast<const char16_t *>(wideString.c_str());
        const char16_t *readEnd = read + wideString.length();

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          UnicodeHelper::WriteCodePoint(write, codePoint);

          if(write + 4 >= writeEnd) [[unlikely]] {
            std::u8string::size_type writeIndex = (write - result.data());
            result.resize(result.size() * 2);
            write = result.data();
            writeEnd = write + result.length();
            write += writeIndex;
          }
        }

        result.resize(write - result.data());
      } else { // Variant for 32 bit wchar_t used everywhere except Windows
        const char32_t *read = reinterpret_cast<const char32_t *>(wideString.c_str());
        const char32_t *readEnd = read + wideString.length();

        while(read < readEnd) {
          UnicodeHelper::WriteCodePoint(write, *read);
          ++read;

          if(write + 4 >= writeEnd) [[unlikely]] {
            std::u8string::size_type writeIndex = (write - result.data());
            result.resize(result.size() * 2);
            write = result.data();
            writeEnd = write + result.length();
            write += writeIndex;
          }
        }

        result.resize(write - result.data());
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u16string StringConverter::Utf16FromUtf8(const std::u8string &utf8String) {
    std::u16string result;
    {
      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      const char8_t *read = utf8String.c_str();
      const char8_t *readEnd = read + utf8String.length();

      char16_t *write = result.data();
      while(read < readEnd) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
        UnicodeHelper::WriteCodePoint(write, codePoint);
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string StringConverter::Utf8FromUtf16(const std::u16string &utf16String) {
    std::u8string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(utf16String.length() * 2);

      char8_t *write = result.data();
      char8_t *writeEnd = write + result.length();

      const char16_t *read = utf16String.c_str();
      const char16_t *readEnd = read + utf16String.length();

      while(read < readEnd) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
        UnicodeHelper::WriteCodePoint(write, codePoint);

        if(write + 4 >= writeEnd) [[unlikely]] {
          std::u8string::size_type writeIndex = (write - result.data());
          result.resize(result.size() * 2);
          write = result.data();
          writeEnd = write + result.length();
          write += writeIndex;
        }
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u32string StringConverter::Utf32FromUtf8(const std::u8string &utf8String) {
    std::u32string result;
    {
      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      const char8_t *read = utf8String.c_str();
      const char8_t *readEnd = read + utf8String.length();

      char32_t *write = result.data();
      while(read < readEnd) {
        *write = UnicodeHelper::ReadCodePoint(read, readEnd);
        ++write;
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string StringConverter::Utf8FromUtf32(const std::u32string &utf32String) {
    std::u8string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(utf32String.length() * 2);

      char8_t *write = result.data();
      char8_t *writeEnd = write + result.length();

      const char32_t *read = utf32String.c_str();
      const char32_t *readEnd = read + utf32String.length();

      while(read < readEnd) {
        UnicodeHelper::WriteCodePoint(write, *read);
        ++read;

        if(write + 4 >= writeEnd) [[unlikely]] {
          std::u8string::size_type writeIndex = (write - result.data());
          result.resize(result.size() * 2);
          write = result.data();
          writeEnd = write + result.length();
          write += writeIndex;
        }
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  void StringConverter::AppendPathAsUtf8(
    std::u8string &target, const std::filesystem::path &pathToAppend
  ) {
    typedef std::filesystem::path::string_type path_string_type;
    if constexpr(
      (std::is_same<path_string_type, std::u8string>::value) ||
      (std::is_same<path_string_type, std::string>::value)
    ) {
      const path_string_type &pathString = pathToAppend.native();
      path_string_type::size_type pathLength = pathString.length();
      std::u8string::size_type targetLength = target.length();

      target.resize(targetLength + pathLength);
      for(path_string_type::size_type index = 0; index < pathLength; ++index) {
        target[targetLength + index] = static_cast<char8_t>(pathString[index]);
      }
    } else {
      target.append(pathToAppend.u8string());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void StringConverter::AppendPathAsWide(
    std::wstring &target, const std::filesystem::path &pathToAppend
  ) {
    typedef std::filesystem::path::string_type path_string_type;
    if constexpr(std::is_same<path_string_type, std::wstring>::value) {
      const path_string_type &pathChars = pathToAppend.native();
      target.append(pathChars.begin(), pathChars.end());
    } else {
      target.append(pathToAppend.wstring());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::u8string StringConverter::FoldedLowercaseFromUtf8(const std::u8string &utf8String) {
    std::u8string result;
    {
      const char8_t *current = utf8String.c_str();
      const char8_t *end;
      {
        std::u8string::size_type length = utf8String.length();
        end = current + length;

        result.resize(length);
      }

      char8_t *target = result.data();

      while(current < end) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
        codePoint = UnicodeHelper::ToFoldedLowercase(codePoint);
        UnicodeHelper::WriteCodePoint(target, codePoint);
      }

      // Remove the excess characters in case the string became shorter
      result.resize(target - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
