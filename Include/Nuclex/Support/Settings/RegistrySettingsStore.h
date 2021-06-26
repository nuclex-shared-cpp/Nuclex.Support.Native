#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_SETTINGS_REGISTRYSETTINGSSTORE_H
#define NUCLEX_SUPPORT_SETTINGS_REGISTRYSETTINGSSTORE_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Settings/SettingsStore.h"

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores application settings in the Windows registry</summary>
  /// <remarks>
  ///   <para>
  ///     With this implementation of the settings store, you can read and write settings
  ///     from and into the Windows registry. The registry is a giant multi-leveled database
  ///     of properties that stores vital operating system data together with application
  ///     specific settings.
  ///   </para>
  ///   <para>
  ///     The registry is not commonly accessed or understood by the user, there is no built-in
  ///     documentation mechanism, it's not portable beyond Windows operating systems and
  ///     you're prone to leave orphaned settings behind when uninstalling. Thus, unless you're
  ///     having specific reason to interface with the registry, it's usually a bad idea that
  ///     will only make your application harder to maintain and harder to port.
  ///   </para>
  ///   <para>
  ///     Any changes made to the settings are immediately reflected in the registry. If you
  ///     need transient changes, you should create a <see cref="MemorySettingsStore" /> and
  ///     copy all settings over, then make the changes in the memory settings store.
  ///   </para>
  /// </remarks>
  class RegistrySettingsStore : public SettingsStore {

    /// <summary>
    ///   Initializes a new registry settings store with settings storing under
    ///   the specified registry key
    /// </summary>
    /// <param name="registryPath">
    ///   Absolute path of the registry key that will be accessed
    /// </param>
    public: NUCLEX_SUPPORT_API RegistrySettingsStore(const std::string &registryPath);

    /// <summary>Frees all resources owned by the .ini settings store</summary>
    public: NUCLEX_SUPPORT_API ~RegistrySettingsStore() override;

    /// <summary>Returns a list of all categories contained in the store</summary>
    /// <returns>A list of all categories present in the store currently</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::string> GetAllCategories() const override;

    /// <summary>Returns a list of all properties found within a category</summary>
    /// <param name="categoryName">Name of the category whose properties will be returned</param>
    /// <returns>A list of all properties present in the specified category</returns>
    /// <remarks>
    ///   If the root level of properties should be listed, pass an empty string as
    ///   the category name. Specifying the name of a category that doesn't exist will
    ///   simply return an empty list (because )
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::vector<std::string> GetAllProperties(
      const std::string &categoryName = std::string()
    ) const override;

    /// <summary>Deletes an entire category with all its properties from the store</summary>
    /// <param name="categoryName">Name of the category that will be deleted</param>
    /// <returns>True if the category existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteCategory(
      const std::string &categoryName
    ) override;

    /// <summary>Deletes the specified property from the store</summary>
    /// <param name="categoryName">
    ///   Name of the category from which the property will be deleted
    /// </param>
    /// <param name="propertyName">Name of the property that will be deleted</param>
    /// <returns>True if the property existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteProperty(
      const std::string &categoryName, const std::string &propertyName
    ) override;

    /// <summary>Retrieves the value of a boolean property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<bool> RetrieveBooleanProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint32_t> RetrieveUInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int32_t> RetrieveInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint64_t> RetrieveUInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int64_t> RetrieveInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a string property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::string> RetrieveStringProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Stores or updates a boolean property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreBooleanProperty(
      const std::string &categoryName, const std::string &propertyName, bool value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::uint32_t value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::int32_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::uint64_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::int64_t value
    ) override;

    /// <summary>Stores or updates a string property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreStringProperty(
      const std::string &categoryName, const std::string &propertyName, const std::string &value
    ) override;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_SETTINGS_REGISTRYSETTINGSSTORE_H
