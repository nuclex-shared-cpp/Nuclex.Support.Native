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

#include "Nuclex/Support/Settings/MemorySettingsStore.h"

#include <memory> // for std::unique_ptr

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  MemorySettingsStore::~MemorySettingsStore() {
    for(
      CategoryMap::const_iterator iterator = this->categories.begin();
      iterator != this->categories.end();
      ++iterator
    ) {
      delete iterator->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> MemorySettingsStore::GetAllCategories() const {
    std::vector<std::string> results;
    results.reserve(this->categories.size()); // Size is an O:1 operation on all imps I checked

    for(
      CategoryMap::const_iterator iterator = this->categories.begin();
      iterator != this->categories.end();
      ++iterator
    ) {
      results.push_back(iterator->first);
    }

    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> MemorySettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::vector<std::string>();
    }

    std::vector<std::string> results;
    results.reserve(categoryIterator->second->size());

    for(
      PropertyMap::const_iterator iterator = categoryIterator->second->begin();
      iterator != categoryIterator->second->end();
      ++iterator
    ) {
      results.push_back(iterator->first);
    }

    return results;
  }

  // ------------------------------------------------------------------------------------------- //

  bool MemorySettingsStore::DeleteCategory(const std::string &categoryName) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return false;
    } else {
      std::unique_ptr<PropertyMap> properties(categoryIterator->second);
      this->categories.erase(categoryIterator);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool MemorySettingsStore::DeleteProperty(
    const std::string &categoryName, const std::string &propertyName
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return false;
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return false;
    } else {
      categoryIterator->second->erase(propertyIterator);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<bool> MemorySettingsStore::RetrieveBooleanProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<bool>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<bool>();
    } else {
      return propertyIterator->second.ToBoolean();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint32_t> MemorySettingsStore::RetrieveUInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<std::uint32_t>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<std::uint32_t>();
    } else {
      return propertyIterator->second.ToUint32();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int32_t> MemorySettingsStore::RetrieveInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<std::int32_t>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<std::int32_t>();
    } else {
      return propertyIterator->second.ToInt32();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint64_t> MemorySettingsStore::RetrieveUInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<std::uint64_t>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<std::uint64_t>();
    } else {
      return propertyIterator->second.ToUint64();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int64_t> MemorySettingsStore::RetrieveInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<std::int64_t>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<std::int64_t>();
    } else {
      return propertyIterator->second.ToInt64();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> MemorySettingsStore::RetrieveStringProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      return std::optional<std::string>();
    }

    PropertyMap::const_iterator propertyIterator = categoryIterator->second->find(propertyName);
    if(propertyIterator == categoryIterator->second->end()) {
      return std::optional<std::string>();
    } else {
      return propertyIterator->second.ToString();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreBooleanProperty(
    const std::string &categoryName, const std::string &propertyName, bool value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreUInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::uint32_t value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::int32_t value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreUInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::uint64_t value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::int64_t value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemorySettingsStore::StoreStringProperty(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
    CategoryMap::const_iterator categoryIterator = this->categories.find(categoryName);
    if(categoryIterator == this->categories.end()) {
      PropertyMap *properties = new PropertyMap();
      this->categories.insert(CategoryMap::value_type(categoryName, properties));
      properties->insert(PropertyMap::value_type(propertyName, Variant(value)));
    } else {
      PropertyMap::const_iterator iterator = categoryIterator->second->find(propertyName);
      if(iterator == categoryIterator->second->end()) {
        categoryIterator->second->insert(PropertyMap::value_type(propertyName, Variant(value)));
      } else {
        categoryIterator->second->insert_or_assign(iterator, propertyName, Variant(value));
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
