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

#include "./StandardServiceCollection.PrivateImplementation.h"

#include <stdexcept> // for std::runtime_error()

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    const std::type_info &serviceType,
    const std::function<std::any(const std::shared_ptr<ServiceProvider> &)> &factory,
    ServiceLifetime lifetime
  ) :
    ServiceType(&serviceType),
    ExistingInstance(), // leave empty
    Factory(factory),
    Lifetime(lifetime) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    const std::type_info &serviceType,
    const std::any &prototype,
    const std::function<std::any(const std::any &)> &cloneFactory,
    ServiceLifetime lifetime
  ) :
    ServiceType(&serviceType),
    ExistingInstance(prototype),
    CloneFactory(cloneFactory),
    Lifetime(ServiceLifetime::Transient) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    const ServiceBinding &other
  ) :
    ServiceType(other.ServiceType),
    ExistingInstance(other.ExistingInstance),
    Lifetime(other.Lifetime) {

    // Anything other than transient binding with an existing instance uses the normal factory
    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    } else {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    ServiceBinding &&other
  ) :
    ServiceType(other.ServiceType),
    ExistingInstance(std::move(other.ExistingInstance)),
    Lifetime(other.Lifetime) {

    // Anything other than transient binding with an existing instance uses the normal factory
    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    } else {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::~ServiceBinding() {
    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      this->Factory.~function();
    } else {
      this->CloneFactory.~function();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding &
  StandardServiceCollection::PrivateImplementation::ServiceBinding::operator =(
    const ServiceBinding &other
  ) {
    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      this->Factory.~function();
    } else {
      this->CloneFactory.~function();
    }

    this->ServiceType = other.ServiceType;
    this->ExistingInstance = other.ExistingInstance;
    this->Lifetime = other.Lifetime;

    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    } else {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding &
  StandardServiceCollection::PrivateImplementation::ServiceBinding::operator =(
    ServiceBinding &&other
  ) {
    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      this->Factory.~function();
    } else {
      this->CloneFactory.~function();
    }

    this->ServiceType = other.ServiceType;
    this->ExistingInstance = std::move(other.ExistingInstance);
    this->Lifetime = other.Lifetime;

    if((this->Lifetime != ServiceLifetime::Transient) || this->ExistingInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    } else {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2
