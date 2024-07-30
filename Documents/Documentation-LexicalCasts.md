Lexical Casts
=============

A lexical cast is a cast that interprets the contents of a string when
converting between strings and numbers. Different functions for achieving
exist in C++ as well, see `std::to_string()`, `std::stoi()` or `std::stof()`.

The `lexical_cast()` function does he same in a design similar to other C++
casts such as `static_cast()` and `reinterpret_cast()`. This is also why
this function's naming convention falls slightly out of line with the rest of
Nuclex.Support.Native.

There is, by design, no localization of number formats with `lexical_cast()`,
it always uses the English locale (meaning the decimal point is a point, not
a comma). It is intended for data conversion in serialization and will always
use the English locale, no matter what the system, process or thread use.


Usage example
-------------

Usage is very simple and identical to Boost. Just replace classic functions
such as `stoi()`, `stof()` or `std::to_string()` with `lexical_cast` where
you wish to be independent of the system's locale (or interested in the speed
boost compared to most C++ standard libraries).

```cpp
#include <Nuclex/Support/Text/LexicalCast.h>

void test() {
  using Nuclex::Support::Text::lexical_cast;

  // Will *always* produce 123.456 independent of the current locale
  std::string numberAsString = lexical_cast<std::string>(123.456f);

  // Will *always* parse correctly independent of the current locale
  float stringAsNumber = lexical_cast<float>("654.321");
}
```

There's also `lexical_append` which will append a number to a string. It will
write the generated characters directly into the std::string:


```cpp
#include <Nuclex/Support/Text/LexicalAppend.h>

void test() {
  using Nuclex::Support::Text::lexical_append;

  std::string scoreLine(u8"Player Score: ");
  lexical_append(scoreLine, 295387);

  appendLineToFile(u8"state.dat", scoreLine);
}
```


Notes
-----

The `lexical_cast` method uses its own code to convert between strings and
numbers, so it doesn't mess with your process' or thread's locale settings.

As for the methods used in the conversion:

- To convert integers to strings, it uses a variant of James Edward Anhalt
  III's technique, which converts two digits at a time. As of 2024, it still
  appears to be the fastest integer to string converter out there:

  - [Reference code on GitHub](https://github.com/jeaiii/itoa)

  - [Nuclex.Support.Native variant](../Source/Text/NumberFormatter-jeaiii.cpp)

- To convert floating point values into strings, it uses DragonBox. This is
  not the same as Dragon4, the traditional (but inefficient) float to string
  algorithm. For a time, Ryu was the fastest, but as of 2024, DragonBox has
  outperformed every other algorithm.

  - [Reference code on GitHub](https://github.com/jk-jeon/dragonbox)

  - [Nuclex.Support.Native variant](../Source/Text/NumberFormatter-dragonbox.cpp)

- String to integer conversion uses the standard `stoi()`, `stof()` and
  related methods from the standard C++ library.

- String to floating point value conversion borrows the `s2d()` method
  provided by Ryu, the formerly fastest floating point to string converter.

In short, it's completely isolated from the string/float conversion code
provided by your standard C++ library or by your operating system.


Scientific Notation / Exponential Number Format
-----------------------------------------------

Currently, `lexical_cast()` and `lexical_append()` avoid scientific notation
for numbers. So If you have a double set to 10 to the power of 300 and convert
that to a string, you'll get a string with a length of 300+ characters.

It will *not* output strings such as `1e300` or `1e-7`.

I an considering to also offer scientific notation, but forcing a simple
format puts fewer requirements on parsing numbers.


Full Round-Trip Guarantee
-------------------------

Of course, full round-trip conversion is guaranteed. If you cast a floating
point value to a string via `lexical_cast` and then back to a floating point
value, the floating point value is guaranteed to be *exactly* the same again,
not just close or rounded to some precision.
