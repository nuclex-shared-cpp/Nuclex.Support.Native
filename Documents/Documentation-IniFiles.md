Ini / Configuration File Settings Store
=======================================

Nuclex.Support.Native contains an efficient and very solid .ini file parser
that implements the `SettingsStore` interface, just like the registry-based
settings store (*windows only*, of course) and the in-memory settings store
for temporary settings or mocking in unit tests.


Features
--------

My goal was to build a parser that would respect user-formatted files and
handle the vast majority of format variations of configuration files.
Here's a summary explaining what this `.ini` file parser / writer can do:

* It can read and write standard format `.ini` and `.conf` files, of course.

* It is fast, robust and it avoids micro-allocations, too.

  *There are unit tests for all code branches and edge cases and,
  just for fun, the parser has been tortured by parsing every file found in
  my Linux file system and inside my Windows VM's `Windows` directory.*

* Const-correct. A const intance of the `IniSettingsStore` only allows
  properties to be retrieved, not stored.

* If properties in an existing `.ini` file are updated, all comments and
  formatting are preserved (including within the lines being updated!)

  ```ini
  # This is an example comment on a section. It might explain something
  # about the purpose of the section or its options.
  [ExampleSection]

  #      | World Values   | Hello Values   | Other Values
  # ---------------------------------------------------------
  Value1 =                  "Hello" # This comment is behind a value
  Value2 = "World"  
  ```

  The following call would non-destructively edit `Hello` into `Bonjour`,
  preserving its indentation and the comment behind it.

  ```cpp
  ini.Store<std::string>(u8"ExampleSection", u8"Value1", u8"Bonjour");
  ```

* If you load an existing `.ini` file, it will analyze its format and use
  the same type of newlines (CRLF / LF), empty lines around properties
  and spaces around the assignment operator for any properties being added.

* Multi-line values are supported. This can be useful if you want to list
  command line parameters or if you have genuine line breaks in your values:

  ```ini
  arguments="
    --deep
    --update
    --newuse
    --with-bdeps=y
    --quiet-build=y
    @world
  "

  # To anyone reading this in the future, it's a 2000s internet meme
  cutsceneSubtitle="All your base
  are belong to us"
  ```

* Property values may contain special characters via escaping:

  ```ini
  setting1 = "\"No, you,\" she said."
  setting2 = "e=mc2"
  setting3 = "C:\\Windows\\System32"
  ```


Loading and Saving `.ini` Files
-------------------------------

If your application needs to create a new `.ini` file, you can simply create
a `Nuclex::Support::Settings::IniSettingsStore` with its default constructor
and begin storing properties in it:

```cpp
IniSettingsStore settings;

settings.Store<std::uint32_t>(u8"MyCategory", u8"SyncIntervalSeconds", 120);
```

If, on the other hand, you want to load an existing `.ini` file, you can
either specify its path in the constructor or use the `Load()` method after
constructing an empty instance (with no overhead):

```cpp
IniSettingsStore settings;

settings.Load(u8"/etc/conf.d/net");

std::optional<std::string> localDnsDomain = settings.Retrieve<std::string>(
  std::string(), u8"dns_domain_lo"
);
```

There are also overloads for both the constructor and the `Load()` method to
load an `.ini` file directly from memory.

Finally, if you want to save the contents of your `.ini` file, the counterpart
to the `Load()` method is, of course, the `Save()` method.

```cpp
IniSettingsStore settings;

settings.Store<std::uint32_t>(u8"MyCategory", u8"SyncIntervalSeconds", 120);

settings.Save(u8"~/.config/my-app.ini");
```

There also is an overload of the `Save()` method that returns an
`std::vector<std::uint8_t>`, in case you are not working with files directly.

Unlike some other implementations, the `IniSettingsStore` class also does not
keep hold of the path from which the `.ini` file was loaded. 


Retrieving and Updating Properties in `.ini` Files
--------------------------------------------------

The above code snippets already revealed the two templated methods,
`Retrieve()` and `Store()`. The methods are defined in the `SettingsStore`
interface, which the `IniSettingsStore` shares with
the `RegistrySettingsStore` and the `MemorySettingsStore`.

```cpp
class SettingsStore {

  // ...

  public: template<typename TValue>
  std::optional<TValue> Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const

  public: template<typename TValue>
  void Store(
    const std::string &categoryName, const std::string &propertyName,
    const TValue &value
  )

  // ...

};
```

If you want to write unit-testable code, or use different ways of storing your
settings between Linux and Windows, you can write your code against the
aforementioned interface instead of accessing the `IniSettingsStore` directly:

```cpp
void updateSavedSettings(SettingsStore &iniOrRegistry) {
  iniOrRegistry.Store<bool>(std::string(), u8"FirstLaunch", false);
}
```

Permitted types (for the template argument) are:

  * `bool`
  * `std::uint32_t`
  * `std::int32_t`
  * `std::uint64_t`
  * `std::int64_t`
  * `std::string`

To store floating point values, convert them into a string yourself. This
keeps the `IniSettingsStore` out of the loop when dealing with
internationalization issues. You can use `Nuclex::Support::Text::lexical_cast`
for conversions to and from `float` or `double` that are guaranteed to use
the `en-US` number format indepentently of the system and thread locale.

You've seen the `Store()` method in action above. The `Retrieve()` method is
its counterpart and returns an `std::optional` type that will be empty in
case the requested property does not exist (so asking for a non-existent
property is acceptable).

You can very conveniently check if the value is present or provide a default
for it via the facilities provided by `std::optional`:

Variant with checking:

```cpp
std::optional<std::uint32_t> screenWidth = ini.Retrieve<std::uint32_t>(
  u8"Resolution", u8"Width"
);

std::optional<std::uint32_t> screenHeight = ini.Retrieve<std::uint32_t>(
  u8"Resolution", u8"Height"
);

if(screenWidth.has_value() && screenHeight.has_value()) {
  changeResolution(screenWidth.value(), screenHeight.value());
}
```

Variant with defaults:

```cpp
std::uint32_t screenWidth = ini.Retrieve<std::uint32_t>(u8"Resolution", u8"Width")
    .value_or(1920);

std::uint32_t screenHeight = ini.Retrieve<std::uint32_t>(u8"Resolution", u8"Height")
    .value_or(1080);

changeResolution(screenWidth, screenHeight);
```


Other Features
--------------

You can also enumerate the sections and properties present in an `.ini` file,
as well as delete individual properties or even whole sections.

```cpp
std::vector<std::string> GetAllCategories() const

std::vector<std::string> GetAllProperties(const std::string &categoryName) const

bool DeleteCategory(const std::string &categoryName)

bool DeleteProperty(
  const std::string &categoryName, const std::string &propertyName
)
```


Error Handling
--------------

In `Nuclex.Support.Native`, error handling uses exceptions.

Regarding the `IniSettingsStore` specifically, however, the parser *will* eat
any file you throw at it without complaint. And you can ask for any property
or store any property, also without complaint.

The only cases where you might see an exception are:

`std::system_error` - when you use the `Load()` or `Save()` methods and either
specify an invalid path or there is a permission isue. `std::system_error` is
the correct type of exception for and will also carry the operating system's
error message (such as "file not found" or "access denied") with it.

`std::bad_alloc` - if the system runs out of memory.

If you request an integer value for a property that has a string value
assigned to it, the string will be interpreted as having a value of `0`.
If you need a different behavior, it is always safe to retrieve any value
as an `std::string` and perform your own conversion by your own rules.
