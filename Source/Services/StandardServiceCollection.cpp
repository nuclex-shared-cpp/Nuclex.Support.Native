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

#include "Nuclex/Support/Services/StandardServiceCollection.h"

#include "./StandardServiceCollection.PrivateImplementation.h"
#include "./StandardServiceProvider.h"
#include "./StandardInstanceSet.h"

#include <stdexcept> // for std::runtime_error()
#include <typeindex> // for std::type_index

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::StandardServiceCollection() :
    privateImplementation(std::make_unique<PrivateImplementation>()) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceCollection::~StandardServiceCollection() = default;

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceProvider> StandardServiceCollection::BuildServiceProvider() const {
    std::shared_ptr<StandardBindingSet> clonedBindingSet = (
      std::make_shared<StandardBindingSet>(this->privateImplementation->Bindings) // copy
    );
    clonedBindingSet->GenerateUniqueIndexes();

    return std::make_shared<StandardServiceProvider>(
      StandardInstanceSet::Create(clonedBindingSet, clonedBindingSet->SingletonServices)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t StandardServiceCollection::RemoveAll(const std::type_info &serviceType) {
    PrivateImplementation &impl = *this->privateImplementation;
    const std::type_index serviceIndex(serviceType);

    return (
      impl.Bindings.SingletonServices.erase(serviceIndex) +
      impl.Bindings.ScopedServices.erase(serviceIndex) +
      impl.Bindings.TransientServices.erase(serviceIndex)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceCollection::AddServiceBinding(
    const std::type_info &serviceType,
    const std::function<std::any(ServiceProvider &)> &factoryMethod,
    ServiceLifetime lifetime
  ) {
    PrivateImplementation &impl = *this->privateImplementation;
    const std::type_index serviceIndex(serviceType);

    // Here we add the service binding to the correct bucket. We also remove all
    // service bindings of the same service from the other buckets each time to ensure
    // a service is only registered with one lifetime policy and later registrations
    // replace the former registrations.
    switch(lifetime) {
      case ServiceLifetime::Singleton: {
        impl.Bindings.SingletonServices.emplace(serviceIndex, factoryMethod);
        impl.Bindings.ScopedServices.erase(serviceIndex);
        impl.Bindings.TransientServices.erase(serviceIndex);
        break;
      }
      case ServiceLifetime::Scoped: {
        impl.Bindings.SingletonServices.erase(serviceIndex);
        impl.Bindings.ScopedServices.emplace(serviceIndex, factoryMethod);
        impl.Bindings.TransientServices.erase(serviceIndex);
        break;
      }
      case ServiceLifetime::Transient: {
        impl.Bindings.SingletonServices.erase(serviceIndex);
        impl.Bindings.ScopedServices.erase(serviceIndex);
        impl.Bindings.TransientServices.emplace(serviceIndex, factoryMethod);
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceCollection::AddPrototypedService(
    const std::type_info &serviceType,
    const std::any &instance,
    const std::function<std::any(const std::any &)> &cloneMethod,
    ServiceLifetime lifetime
  ) {
    PrivateImplementation &impl = *this->privateImplementation;
    const std::type_index serviceIndex(serviceType);

    // Here we add the service binding to the correct bucket. We also remove all
    // service bindings of the same service from the other buckets each time to ensure
    // a service is only registered with one lifetime policy and later registrations
    // replace the former registrations.
    switch(lifetime) {
      case ServiceLifetime::Singleton: {
        impl.Bindings.SingletonServices.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(serviceIndex),
          std::forward_as_tuple(instance, cloneMethod)
        );
        impl.Bindings.ScopedServices.erase(serviceIndex);
        impl.Bindings.TransientServices.erase(serviceIndex);
        break;
      }
      case ServiceLifetime::Scoped: {
        impl.Bindings.SingletonServices.erase(serviceIndex);
        impl.Bindings.ScopedServices.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(serviceIndex),
          std::forward_as_tuple(instance, cloneMethod)
        );
        impl.Bindings.TransientServices.erase(serviceIndex);
        break;
      }
      case ServiceLifetime::Transient: {
        impl.Bindings.SingletonServices.erase(serviceIndex);
        impl.Bindings.ScopedServices.erase(serviceIndex);
        impl.Bindings.TransientServices.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(serviceIndex),
          std::forward_as_tuple(instance, cloneMethod)
        );
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
