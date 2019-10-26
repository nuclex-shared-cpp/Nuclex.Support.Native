#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#include "Nuclex/Support/Services/LazyServiceInjector.h"
#include "Nuclex/Support/Services/UnresolvedDependencyError.h"

#include <stdexcept>

// TODO: Create a ServiceConstructionChain or something to detect cyclic dependencies

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  const Any &LazyServiceInjector::Get(const std::type_info &serviceType) const {

    // Check if the service has already been constructed
    ServiceInstanceMap::iterator instanceIt = this->instances.find(&serviceType);
    if(instanceIt != this->instances.end()) {
      return instanceIt->second;
    }

    // Check if a factory for the service has been registered
    ServiceFactoryMap::const_iterator factoryIt = this->factories.find(&serviceType);
    if(factoryIt != this->factories.end()) {
      const ServiceFactoryType &serviceFactory = factoryIt->second;
      if(serviceFactory.first == ServiceLifetime::Request) {
        throw std::runtime_error("Per-request services can only be obtained via Create()");
      }

      // Create the service instance and store it in the instance map
      std::pair<ServiceInstanceMap::iterator, bool> result = this->instances.insert(
        ServiceInstanceMap::value_type(&serviceType, serviceFactory.second(*this))
      );
      
      instanceIt = result.first;
      return instanceIt->second;
    }

    // We could attempt an ad-hoc service creation here, but there are several concerns
    // speaking against doing so: a) we don't have the type in template form anymore,
    // b) the service is not registered as a container singleton and creating a per-request
    // service would be confusing.
    std::string message = "Service '";
    message += serviceType.name();
    message += " is not known to the injector. Please register it before requesting.";
    throw UnresolvedDependencyError(message);

  }

  // ------------------------------------------------------------------------------------------- //

  Any LazyServiceInjector::Create(const std::type_info &serviceType) const {

    // Check if the service has already been constructed
    ServiceInstanceMap::iterator instanceIt = this->instances.find(&serviceType);
    if(instanceIt != this->instances.end()) {
      return instanceIt->second;
    }

    // Check if a factory for the service has been registered
    ServiceFactoryMap::const_iterator factoryIt = this->factories.find(&serviceType);
    if(factoryIt != this->factories.end()) {
      const ServiceFactoryType &serviceFactory = factoryIt->second;
      if(serviceFactory.first == ServiceLifetime::Request) {
        return serviceFactory.second(*this);
      }

      // Create the service instance and store it in the instance map
      std::pair<ServiceInstanceMap::iterator, bool> result = this->instances.insert(
        ServiceInstanceMap::value_type(&serviceType, serviceFactory.second(*this))
      );
      
      instanceIt = result.first;
      return instanceIt->second;
    }

    // We could attempt an ad-hoc service creation here, but there are several concerns
    // speaking against doing so: a) we don't have the type in template form anymore,
    // b) the service is not registered as a container singleton and creating a per-request
    // service would be confusing.
    std::string message = "Service '";
    message += serviceType.name();
    message += " is not known to the injector. Please register it before requesting.";
    throw UnresolvedDependencyError(message);

  }

  // ------------------------------------------------------------------------------------------- //

  const Any &LazyServiceInjector::TryGet(const std::type_info &serviceType) const {
    ServiceInstanceMap::iterator instanceIt = this->instances.find(&serviceType);
    if(instanceIt == this->instances.end()) {
      return Any::Empty;
    } else {
      return instanceIt->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
