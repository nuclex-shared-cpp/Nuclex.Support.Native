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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Settings/IniSettingsStore.h"

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore() {}

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore(const std::string &iniFilePath) {
    Load(iniFilePath);
  }

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore(
    const std::uint8_t *iniFileContents, std::size_t iniFileByteCount
  ) {
    Load(iniFileContents, iniFileByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::~IniSettingsStore() {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Load(const std::string &iniFilePath) {

  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Load(const std::uint8_t *iniFileContents, std::size_t iniFileByteCount) {

  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Save(const std::string &iniFilePath) const {

  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::uint8_t> IniSettingsStore::Save() const {

  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::HasChangedSinceLoad() const {

  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniSettingsStore::GetAllCategories() const {
    std::vector<std::string> results;
    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniSettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    std::vector<std::string> results;
    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::DeleteCategory(const std::string &categoryName) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::DeleteProperty(
    const std::string &categoryName, const std::string &propertyName
  ) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<bool> IniSettingsStore::RetrieveBooleanProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<bool>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint32_t> IniSettingsStore::RetrieveUInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::uint32_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int32_t> IniSettingsStore::RetrieveInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::int32_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint64_t> IniSettingsStore::RetrieveUInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::uint64_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int64_t> IniSettingsStore::RetrieveInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::int64_t>();
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> IniSettingsStore::RetrieveStringProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return std::optional<std::string>();
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreBooleanProperty(
    const std::string &categoryName, const std::string &propertyName, bool value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreUInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::uint32_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::int32_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreUInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::uint64_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::int64_t value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreStringProperty(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
