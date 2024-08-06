Unicode Iteration, Decoding and Encoding
========================================

This is an extra lightweight helper class for dealing with unicode encoding
in UTF-8 and UTF-16.


Alternative: UTFcpp
-------------------

There's a popular C++ library called utfcpp by Nemanja Trifunovic (and earlier
versions of Nuclex.Support.Native even used it) which is popular for this task:

[Original Presentation of the UTFcpp header](https://www.codeproject.com/Articles/14637/UTF-8-With-C-in-a-Portable-Way)

[Current home of UTFcpp on GitHub](https://github.com/nemtrif/utfcpp)

The `UnicodeHelper` in Nuclex.Support.Native goes for a firmer, more C-styled
interface to tackle the issue. When dealing with UTF-8 on the byte/character
level, my code is usually low-level - loops and checks - where I find that
this harmonizes better with the usage patterns present there. Other than that,
there is a lot of overlap with UTFcpp and UTFcpp is more feature-rich.


Unicode Crash Course
--------------------

For those who need a little refresher, this is what it's all about:

- Up until the mid-1980s, computers overwhelmingly used english interfaces
  and text was stored in ASCII. ASCII needed one byte per letter and defined
  127 characters (so only 7 of the 8 bits in a byte were used).

- For other languages, it was common that each application did its own thing.
  By 1987 a group of three people had the idea to collect all characters from
  every language in the world into one big table.

  As you might expect, there are more than 256 characters in the world.
  More than 65536 (16 bit int), too. An index into this all-encompassing
  "unicode" table requires a 32-bit integer.

- UTF-32: analogous to ASCII, you could store unicode text by simply saving
  each unicode character's 32-bit index in the unicode table. That happens to
  be exactly what UTF-32 does: one 32-bit integer = one unicode character.

  UTF-16 uses 16-bit integers instead. For unicode character indices above
  the 16-bit integer range, it uses special values (between 0xdc00 and 0xe000)
  that mean that the next 16-bit integer is not a character on its own,
  but extends the current character's bits.

  UTF-8, finally, is includes, unchanged, ASCII (0 - 127) and uses the upper
  bit(s) in fun ways to indicate how many bytes combine into a single unicode
  character index. UTF-8 is space efficient, backwards compatible and
  the standard everywhere (except for Windows OS APIs).

- Unicode uses some terms slightly differently than you may be used to:

  * A character now means the smallest encoding unit (so a byte / octet for
    UTF-8 or a 16-bit integer for UTF-16).

  * Letters or glyphs are called code points (because they're an index into
    the unicode table)

- Microsoft did its own thing. They used character tables from ANSI and worked
  them into their own "code pages." Some code pages use 1 byte per letter,
  some use 2 or more in a variable-length encoding.

  Microsoft never dared to go all-in on UTF-8, but they adopted UTF-16 (after
  trying to create their own, slightly incompatible variant called UCS-2), so
  for Windows programmers, unicode has become synonymous with the crutches it
  demands of them: `wchar_t`, `TEXT()` macros and even an entire parallel
  Windows API where all function names end in a `W` (like `CreateWindowA()`
  for the code page variant and `CreateWindowW()` for the UTF-16 variant).

There's a lot of explanation in `UnicodeHelper.h` itself, too.


Reading Code Points
-------------------

Assuming you have a UTF-8 string, you can read the next character like this:

```cpp
using Nuclex::Support::Text::UnicodeHelper;

void enumerateCodePoints(const std::string &utf8String) {
  UnicodeHelper::Char8Type *start = reinterpret_cast<UnicodeHelper::Char8Type>(
    utf8String.data()
  );
  UnicodeHelper::Char8Type *end = start + utf8String.length();

  while(start < end) {
    char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
    if(codePoint == char32_t(-1)) {
      // ... code point is invalid or truncated ...
    } else {
      // ... do something with the validated UTF-32 code point ...
    }
  }
}
```

The `ReadCodePoint()` method advances the read pointer by one code point, so between 1 and 4
characters / bytes.

If a code point seems invalid (i.e. there are not enough bytes remaining to decode it or it
begins with a trailing byte or any other such error), -1 will be returned and the read pointer
will not be advanced at all.

You should absolutely check for this, if only to avoid ending up in an infinite loop.

The same can be done for UTF-16 inputs:

```cpp
using Nuclex::Support::Text::UnicodeHelper;

void enumerateCodePoints(const std::u16string &utf16String) {
  char16_t *start = utf16String.data();
  char16_t *end = start + utf16String.length();

  while(start < end) {
    char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
    if(codePoint == char32_t(-1)) {
      // ... code point is invalid or truncated ...
    } else {
      // ... do something with the validated UTF-32 code point ...
    }
  }
}
```

At the time I'm writing this, `std::u8string` and `char8_t` from the C++20 standard are
not yet used in Nuclex.Support.Native.

You can rely on `UnicodeHelper::Char8Type` to be the correct type, before and after C++20
adoption takes place. In C++17, it will be `std::uint8_t` and in C++20 it will eventually
become `char8_t`.


Writing Code Points
-------------------

Analogous to the `ReadCodePoint()` method, there is also a `WriteCodePoint()`
method that transforms `char32_t` unicode characters (UTF-32) into either
UTF-8 or UTF-16 characters:

```cpp
using Nuclex::Support::Text::UnicodeHelper;

std::string generateUtf8String() {
  std::string result;

  // Make enough room in the string. The maximum possible size per code point
  // is 4 chars, so for the 5 code points below, we need up to 5 * 4 = 20 chars.
  result.resize(20);

  UnicodeHelper::Char8Type *startPointer = (
    reinterpret_cast<UnicodeHelper::Char8Type *>(result.data())
  );
  UnicodeHelper::Char8Type *writePointer = startPointer;

  // Write "Hello" in Japanese hiragana as UTF-8 into the string.
  // U'' in C++ = a char32_t containing the UTF-32 code point.
  UnicodeHelper::WriteCodePoint(writePointer, U'こ');
  UnicodeHelper::WriteCodePoint(writePointer, U'ん');
  UnicodeHelper::WriteCodePoint(writePointer, U'に');
  UnicodeHelper::WriteCodePoint(writePointer, U'ち');
  UnicodeHelper::WriteCodePoint(writePointer, U'は');

  // Now shrink the string to the actual number of chars we wrote
  result.resize(writePointer - startPointer);
  return result;
}
```

Just like `ReadCodePoint()`, the `WriteCodePoint()` method moves the write
pointer forward, this time by the number of characters written. There is
no bounds checking here since the maximum number of characters that can be
generated is at most 4.

You can either make sure that there are 4 characters available behind
the write pointer, or use `CountUtf8Characters()` to obtain the actual number
of characters that will be written (which does a range check with 4 `if`
statements, so it's not slow by any means, it just involves extra branching).

This keeps all your options open. Depending on your scenario you can use
different allocation approaches. Here is some inspiration:

  - You could: allocate 4 chars for each UTF-32 code point you wish to write.

    It's fast due to requiring no bounds checking and works well
    for either short strings or if you have a larger buffer anyway.

  - You could: count the number of chars required up front by calling
    `CountUtf8Characters()` for each code point you are going to write.

    Useful in streaming scenarios with long strings or if allocations are
    what you are most concerned about.

  - You could: allocate in steps. For example, ensure that there are 32 chars
    of available space, then blindly process 8 code points, then check again.

    This method is relatively efficient and you can mix it with guessing.
    If your application processes mostly english text, start by allocating
    the same number of chars as you have code points. If your application is
    international, start by allocating two times that number of code points.
