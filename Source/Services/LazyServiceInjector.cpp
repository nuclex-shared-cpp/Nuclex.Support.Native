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

#include "Nuclex/Support/Services/LazyServiceInjector.h"
#include "Nuclex/Support/Errors/UnresolvedDependencyError.h"

#include <stdexcept>

// TODO: Create a ServiceConstructionChain or something to detect cyclic dependencies

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>An any instance that does not carry any value</summary>
  static const std::any EmptyAny;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  const std::any &LazyServiceInjector::Get(const std::type_info &serviceType) const {

    // Check if the service has already been constructed
    ServiceInstanceMap::iterator instanceIt = this->instances.find(&serviceType);
    if(instanceIt != this->instances.end()) {
      return instanceIt->second;
    }

    // Check if a factory for the service has been registered
    ServiceFactoryMap::const_iterator factoryIt = this->factories.find(&serviceType);
    if(factoryIt != this->factories.end()) {

      // Create the service instance and store it in the instance map
      std::pair<ServiceInstanceMap::iterator, bool> result = this->instances.insert(
        ServiceInstanceMap::value_type(&serviceType, factoryIt->second(*this))
      );
      //this->factories.erase(factoryIt);

      instanceIt = result.first;
      return instanceIt->second;
    }

    // We could attempt an ad-hoc service creation here, but there are several concerns
    // speaking against doing so: a) we don't have the type in template form anymore,
    // b) the service is not registered as a container singleton and creating a per-request
    // service would be confusing.
    std::string message = u8"Service '";
    message += serviceType.name();
    message += u8" is not known to the injector. Please register it before requesting.";
    throw Errors::UnresolvedDependencyError(message);

  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &LazyServiceInjector::TryGet(const std::type_info &serviceType) const {

    // Check if the service has already been constructed
    ServiceInstanceMap::iterator instanceIt = this->instances.find(&serviceType);
    if(instanceIt != this->instances.end()) {
      return instanceIt->second;
    }

    // Check if a factory for the service has been registered
    ServiceFactoryMap::const_iterator factoryIt = this->factories.find(&serviceType);
    if(factoryIt != this->factories.end()) {

      // Create the service instance and store it in the instance map
      std::pair<ServiceInstanceMap::iterator, bool> result = this->instances.insert(
        ServiceInstanceMap::value_type(&serviceType, factoryIt->second(*this))
      );
      this->factories.erase(factoryIt);

      instanceIt = result.first;
      return instanceIt->second;
    }

    // Could not resolve, so return nothing
    return EmptyAny;

  }

  // ------------------------------------------------------------------------------------------- //

  std::any LazyServiceInjector::Create(const std::type_info &serviceType) const {

    // Check if a factory for the service has been registered
    ServiceFactoryMap::const_iterator factoryIt = this->factories.find(&serviceType);
    if(factoryIt != this->factories.end()) {
      return factoryIt->second(*this);
    }

    // We could attempt an ad-hoc service creation here, but there are several concerns
    // speaking against doing so: a) we don't have the type in template form anymore,
    // b) the service is not registered as a container singleton and creating a per-request
    // service would be confusing.
    std::string message = u8"Service '";
    message += serviceType.name();
    message += u8" is not known to the injector. Please register it before requesting.";
    throw Errors::UnresolvedDependencyError(message);

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
