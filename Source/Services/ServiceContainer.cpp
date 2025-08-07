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

#include "Nuclex/Support/Services/ServiceContainer.h"

#include <stdexcept> // for std::logic_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An any instance that does not carry any value</summary>
  static const std::any EmptyAny;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  const std::any &ServiceContainer::Get(const std::type_info &serviceType) const {
    const static std::u8string serviceOfType(u8"Service of type '", 17);
    const static std::u8string notPresent(u8"' not present", 13);

    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      std::string message;
      message.reserve(17 + 32 + 13);
      message.append(serviceOfType.begin(), serviceOfType.end());
      message.append(serviceType.name());
      message.append(notPresent.begin(), notPresent.end());
      throw std::logic_error(message);
    }

    return iterator->second;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &ServiceContainer::TryGet(const std::type_info &serviceType) const {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      return EmptyAny;
    } else {
      return iterator->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ServiceContainer::Add(const std::type_info &serviceType, const std::any &service) {
    const static std::u8string serviceTypeString(u8"Service type '", 14);
    const static std::u8string alreadyAdded(u8"' already added", 15);

    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator != this->services.end()) {
      std::string message;
      message.reserve(14 + 32 + 15);
      message.append(serviceTypeString.begin(), serviceTypeString.end());
      message.append(serviceType.name());
      message.append(alreadyAdded.begin(), alreadyAdded.end());
      throw std::logic_error(message);
    }

    this->services.insert(ServiceMap::value_type(&serviceType, service));
  }

  // ------------------------------------------------------------------------------------------- //

  bool ServiceContainer::Remove(const std::type_info &serviceType) {
    ServiceMap::const_iterator iterator = this->services.find(&serviceType);
    if(iterator == this->services.end()) {
      return false;
    } else {
      this->services.erase(iterator);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
