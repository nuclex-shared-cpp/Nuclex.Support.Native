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

#ifndef NUCLEX_SUPPORT_SETTINGS_INISETTINGSSTORE_H
#define NUCLEX_SUPPORT_SETTINGS_INISETTINGSSTORE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Settings/SettingsStore.h"

#include <vector> // for std::vector
#include <filesystem> // for std::filesystem::path

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores application settings in an .ini / .cfg file</summary>
  /// <remarks>
  ///   <para>
  ///     This implementation of the settings store reads and writes traditional .ini files.
  ///     It does its very best to be non-destructive, meaning that ordering, formatting
  ///     and comments in .ini files are preserved even when they are updated.
  ///   </para>
  ///   <para>
  ///     Using .ini files is the recommended way to store local application configuration
  ///     because it's easy to understand, portable and makes it simple to copy configurations
  ///     around and fully delete an application without potentially leaving unwanted stuff
  ///     behind as would be the case with some alternatives such as the Windows registry.
  ///   </para>
  ///   <para>
  ///     Do notice that this implementation does not automatically update the file on disk
  ///     when values change. You will have to call <see cref="Save" /> upon completing your
  ///     changes or before exiting the application. To aid you in deciding whether this is
  ///     necessary, the <see cref="HasChangedSinceLoading" /> method is provided.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       void test() {
  ///         IniSettingsStore settings(u8"GameSettings.ini");
  ///
  ///         // Retrieve() returns std::optional&lt;T&gt;, so you can either
  ///         // check if the value was present with .has_value() and .value() or
  ///         // directly provide a default via .value_or() as shown below
  ///
  ///         std::uint32_t resolutionX = settings.Retrieve&lt;std::uint32_t&gt;(
  ///           u8"Video", u8"ResolutionX").value_or(1920)
  ///         );
  ///         std::uint32_t resolutionY = settings.Retrieve&lt;std::uint32_t&gt;(
  ///           u8"Video", u8"ResolutionY").value_or(1080)
  ///         );
  ///
  ///         settings.Store&lt;bool&gt;(std::u8string(), u8"FirstLaunch", false);
  ///
  ///         settings.Save(u8"GameSettings.ini");
  ///       }
  ///     </code>
  ///   </example>
  ///   <para>
  ///     Figuring out the path in which the look for/store an .ini file is not
  ///     covered by this class. One options is to use the Nuclex.Support.Storage
  ///     library to determine the paths to your application's executable directory,
  ///     data directory or user settings directory. Another option would be to use
  ///     the <see cref="Nuclex.Support.Threading.Process.GetExecutableDirectory" />
  ///     method, but not that storing .ini files in your application directory is
  ///     not a good idea for cross-platform development.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE IniSettingsStore : public SettingsStore {

    /// <summary>Initializes a new .ini settings store with no backing file</summary>
    /// <remarks>
    ///   The configuration may be saved as an .ini file at a later point in time by
    ///   using the <see cref="Save" /> method. If you only need a transient settings store,
    ///   you should prefer the <see cref="MemorySettingsStore" /> as it's much faster
    ///   while also reducing processing overhead.
    /// </remarks>
    public: NUCLEX_SUPPORT_API IniSettingsStore();

    /// <summary>
    ///   Initializes a new .ini settings store with settings from the specified file
    /// </summary>
    /// <param name="iniFilePath">Absolute path to the .ini file that will be loaded</param>
    public: NUCLEX_SUPPORT_API IniSettingsStore(const std::u8string &iniFilePath);

    /// <summary>
    ///   Initializes a new .ini settings store with settings loaded from an .ini file
    ///   that has already been copied to memory
    /// </summary>
    /// <param name="iniFileContents">Contents of the .ini file in memory</param>
    /// <param name="iniFileByteCount">Total length of the .ini file in bytes</param>
    public: NUCLEX_SUPPORT_API IniSettingsStore(
      const std::byte *iniFileContents, std::size_t iniFileByteCount
    );

    /// <summary>Frees all resources owned by the .ini settings store</summary>
    public: NUCLEX_SUPPORT_API ~IniSettingsStore() override;

    /// <summary>Loads the settings from an .ini file</summary>
    /// <param name="iniFilePath">Absolute path to the .ini file that will be loaded</param>
    public: NUCLEX_SUPPORT_API void Load(const std::filesystem::path &iniFilePath);

    /// <summary>
    ///   Loads the settings from an .ini file that has already been copied into memory
    /// </summary>
    /// <param name="iniFileContents">Contents of the .ini file in memory</param>
    /// <param name="iniFileByteCount">Total length of the .ini file in bytes</param>
    public: NUCLEX_SUPPORT_API void Load(
      const std::byte *iniFileContents, std::size_t iniFileByteCount
    );

    /// <summary>Saves the settings into an .ini file with the specified name</summary>
    /// <param name="iniFilePath">Absolute path where the .ini file will be saved</param>
    public: NUCLEX_SUPPORT_API void Save(const std::filesystem::path &iniFilePath) const;

    /// <summary>Saves the settings into an .ini file that is created in memory</summary>
    /// <returns>A memory block holding the file contents of the .ini file</returns>
    public: NUCLEX_SUPPORT_API [[nodiscard]] std::vector<std::byte> Save() const;

    /// <summary>Checks if any settings have changed since the .ini file was loaded</summary>
    /// <returns>True if the settings were modified, false if no changes were made</returns>
    public: NUCLEX_SUPPORT_API bool HasChangedSinceLoad() const;

    /// <summary>Returns a list of all categories contained in the store</summary>
    /// <returns>A list of all categories present in the store currently</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::u8string> GetAllCategories() const override;

    /// <summary>Returns a list of all properties found within a category</summary>
    /// <param name="categoryName">Name of the category whose properties will be returned</param>
    /// <returns>A list of all properties present in the specified category</returns>
    /// <remarks>
    ///   If the root level of properties should be listed, pass an empty string as
    ///   the category name. Specifying the name of a category that doesn't exist will
    ///   simply return an empty list (because )
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::vector<std::u8string> GetAllProperties(
      const std::u8string &categoryName = std::u8string()
    ) const override;

    /// <summary>Deletes an entire category with all its properties from the store</summary>
    /// <param name="categoryName">Name of the category that will be deleted</param>
    /// <returns>True if the category existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteCategory(
      const std::u8string &categoryName
    ) override;

    /// <summary>Deletes the specified property from the store</summary>
    /// <param name="categoryName">
    ///   Name of the category from which the property will be deleted
    /// </param>
    /// <param name="propertyName">Name of the property that will be deleted</param>
    /// <returns>True if the property existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteProperty(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) override;

    /// <summary>Retrieves the value of a boolean property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<bool> RetrieveBooleanProperty(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint32_t> RetrieveUInt32Property(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int32_t> RetrieveInt32Property(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint64_t> RetrieveUInt64Property(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int64_t> RetrieveInt64Property(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a string property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::u8string> RetrieveStringProperty(
      const std::u8string &categoryName, const std::u8string &propertyName
    ) const override;

    /// <summary>Stores or updates a boolean property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreBooleanProperty(
      const std::u8string &categoryName, const std::u8string &propertyName, bool value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt32Property(
      const std::u8string &categoryName, const std::u8string &propertyName, std::uint32_t value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt32Property(
      const std::u8string &categoryName, const std::u8string &propertyName, std::int32_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt64Property(
      const std::u8string &categoryName, const std::u8string &propertyName, std::uint64_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt64Property(
      const std::u8string &categoryName, const std::u8string &propertyName, std::int64_t value
    ) override;

    /// <summary>Stores or updates a string property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreStringProperty(
      const std::u8string &categoryName, const std::u8string &propertyName, const std::u8string &value
    ) override;

    /// <summary>Hidden document model and formatting informations</summary>
    private: struct PrivateImplementationData;
    /// <summary>Hidden implementation details only required internally</summary>
    private: PrivateImplementationData *privateImplementationData;
    /// <summary>Whether the settings have been modified since they were loaded</summary>
    private: bool modified;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_MEMORYSETTINGSSTORE_H
