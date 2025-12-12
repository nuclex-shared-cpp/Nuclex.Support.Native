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
    Prototype(), // leave empty
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
    Prototype(prototype),
    CloneFactory(cloneFactory),
    Lifetime(ServiceLifetime::Transient) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    const ServiceBinding &other
  ) :
    ServiceType(other.ServiceType),
    Prototype(other.Prototype),
    Lifetime(other.Lifetime) {

    if(this->Prototype.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    } else {
      new(&this->Factory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::ServiceBinding(
    ServiceBinding &&other
  ) :
    ServiceType(other.ServiceType),
    Prototype(std::move(other.Prototype)),
    Lifetime(other.Lifetime) {

    if(this->Prototype.has_value()) {
      new(&this->CloneFactory) std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    } else {
      new(&this->Factory) std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding::~ServiceBinding() {
    if(this->Prototype.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding &
  StandardServiceCollection::PrivateImplementation::ServiceBinding::operator =(
    const ServiceBinding &other
  ) {
    if(this->Prototype.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }

    this->ServiceType = other.ServiceType;
    this->Prototype = other.Prototype;
    this->Lifetime = other.Lifetime;

    if(this->Prototype.has_value()) {
      new(&this->CloneFactory) std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    } else {
      new(&this->Factory) std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    }

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::PrivateImplementation::ServiceBinding &
  StandardServiceCollection::PrivateImplementation::ServiceBinding::operator =(
    ServiceBinding &&other
  ) {
    if(this->Prototype.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }

    this->ServiceType = other.ServiceType;
    this->Prototype = std::move(other.Prototype);
    this->Lifetime = other.Lifetime;

    if(this->Prototype.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    } else {
      new(&this->Factory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    }

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2
