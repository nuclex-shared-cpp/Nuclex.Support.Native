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

#include "./StandardServiceProvider.h"
#include "./StandardInstanceSet.h"

#include "Nuclex/Support/Errors/UnresolvedDependencyError.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::ResolutionContext(
    const std::shared_ptr<StandardInstanceSet> &instanceSet
  ) :
    instanceSet(instanceSet) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::~ResolutionContext() = default;

  // ------------------------------------------------------------------------------------------- //
/*
  std::size_t StandardServiceProvider::ResolutionContext::GetResolutionDepth() const {
    // TODO: Implement StandardServiceProvider::ResolutionContext::GetResolutionDepth() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }
*/
  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceProvider::ResolutionContext::CreateScope() {
    // TODO: Implement StandardServiceProvider::ResolutionContext::CreateScope() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::TryGetService(
    const std::type_info &serviceType
  ) {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::ResolutionContext::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::GetService(
    const std::type_info &serviceType
  ) {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::ResolutionContext::GetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceProvider::ResolutionContext::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::ResolutionContext::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceProvider::ResolutionContext::GetServices(
    const std::type_info &serviceType
  ) {
    // TODO: Implement StandardServiceProvider::ResolutionContext::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::StandardServiceProvider(
    std::shared_ptr<StandardInstanceSet> &&services
  ) :
    services(services) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::~StandardServiceProvider() = default;

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceProvider::CreateScope() {
    // TODO: Implement StandardServiceProvider::CreateScope() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::TryGetService(const std::type_info &serviceType) {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::GetService(const std::type_info &serviceType) {
    const std::type_index serviceIndex(serviceType);

    // First, look for a singleton service that we can deliver. Even if the user has
    // transient services registered, the only reason to go through the dependency injector
    // with them is if they have service dependencies, so the majority of calls to
    // this method will be looking for a singleton service.
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      this->services->OwnBindings.find(serviceIndex)
    );
    if(serviceIterator != this->services->OwnBindings.end()) [[likely]] {
      return fetchOrActivateSingletonService(serviceIterator);
    }

    // It was not a registered singleton service. So next, we'll check if it is
    // a transient service. These services can be requested at any level.
    serviceIterator = this->services->Bindings->TransientServices.find(serviceIndex);
    if(serviceIterator != this->services->Bindings->TransientServices.end()) [[likely]] {
      return cloneTransientService(serviceIterator);
    }

    // At this point, we know the service cannot be provided by the root level service
    // provider. However, as a small courtesy to the user, we'll look in the scoped services
    // to provide a helpful error message if it appears that the user tried to request
    // a scoped services from the root level service provider.
    serviceIterator = this->services->Bindings->ScopedServices.find(serviceIndex);
    if(serviceIterator == this->services->Bindings->TransientServices.end()) [[likely]] {
      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceType.name());
      message.append(serviceType.name(), serviceType.name() + length);
      message.append(u8"' (name may be mangled) has not been registered", 47);
      throw Errors::UnresolvedDependencyError(message);
    } else {
      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceType.name());
      message.append(serviceType.name(), serviceType.name() + length);
      message.append(
        u8"' (name may be mangled) is a scoped service and cannot be requested from "
        u8"the root-level service provider",
        104
      );
      throw Errors::UnresolvedDependencyError(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceProvider::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceProvider::GetServices(const std::type_info &serviceType) {
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::fetchOrActivateSingletonService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's not need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->services->Instances[uniqueServiceIndex];
    }

    // As of a moment ago, a service instance had not been created yet, so acquire
    // the mutex, re-check and create the service instance if needed
    {
      std::unique_lock<std::mutex> changeMutexLocklScope(this->services->ChangeMutex);

      // Before entering the mutex, no instance of the service has been created. However,
      // another thread could have been faster, so check again from inside the mutex were
      // only one thread can enter at a time. This ensures the service is only constructed
      // once and not modified while other threads are in the process of fetching it.
      bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
        std::memory_order::consume
      );
      if(isAlreadyCreated) [[likely]] {
        return this->services->Instances[uniqueServiceIndex];
      }

      // Service now definitely needs to be created. So now set up a resolution context
      // (a proxy around the service provider to detect cyclic dependencies) and start
      // creating any required services down the dependency graph.

      // If an existing instance was provided, just put it in place without worrying about it
      if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
        this->services->Instances[uniqueServiceIndex] = serviceIterator->second.ProvidedInstance;
      } else {
        ResolutionContext context(this->services);
        this->services->Instances[uniqueServiceIndex] = serviceIterator->second.Factory(context);
      }

      this->services->PresenceFlags[uniqueServiceIndex].store(
        true, std::memory_order::release
      );
    } // mutex lock

    return this->services->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::cloneTransientService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    return serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance);
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2
