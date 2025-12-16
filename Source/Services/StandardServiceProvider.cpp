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
#include "Nuclex/Support/Errors/CyclicDependencyError.h"
#include "Nuclex/Support/ScopeGuard.h" // for ON_SCOPE_EXIT {}

#include "./StandardServiceScope.h"

#include <algorithm> // for std::find
#include <iterator> // for std::prev
#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Finds the last service binding for a specified service type</summary>
  /// <param name="services">Service bindings that will be searched</param>
  /// <param name="serviceTypeIndex">
  ///   <code>std::type_index</code> of the service type that will be searched
  /// </param>
  /// <returns>
  ///   An iterator that points either to the last service binding for the specified type
  ///   or to the end of the service binding map if not found
  /// </returns>
  /// <remarks>
  ///   As per convenient, when looking up individual services, the latest registered
  ///   service overrides any earlier registrations. This wrapper method ensures that behavior.
  /// </remarks>
  Nuclex::Support::Services::StandardBindingSet::TypeIndexBindingMultiMap::const_iterator
  findLast(
    const Nuclex::Support::Services::StandardBindingSet::TypeIndexBindingMultiMap &services,
    const std::type_index &serviceTypeIndex
  ) {
    using Nuclex::Support::Services::StandardBindingSet;
    std::pair<
      StandardBindingSet::TypeIndexBindingMultiMap::const_iterator,
      StandardBindingSet::TypeIndexBindingMultiMap::const_iterator
    > range = services.equal_range(serviceTypeIndex);

    if(range.first == range.second) {
      return services.end();
    }

    return std::prev(range.second);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::ResolutionContext(
    StandardInstanceSet &services
  ) :
    services(services),
    mutexAcquired(false),
    resolutionStack() {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::~ResolutionContext() {
    if(this->mutexAcquired.load(std::memory_order::consume)) {
      this->services.ChangeMutex.unlock();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceProvider::ResolutionContext::CreateScope() {
    // This would make no sense. Any service scope created inside of a service factory
    // would have to be gone by the time service resolution finishes (unless you involve
    // global variables or state passed through lambdas).
    #if 0
    return std::make_shared<StandardServiceScope>(
      StandardInstanceSet::Create(
        this->services->Bindings, this->services->Bindings->ScopedServices
      ),
      this->services
    );
    #endif

    throw std::logic_error(
      reinterpret_cast<const char *>(u8"Cannot create scopes from a service factory")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::TryGetService(
    const std::type_info &serviceType
  ) {
    // TODO: Implement StandardServiceProvider::ResolutionContext::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::GetService(
    const std::type_info &serviceType
  ) {
    std::type_index serviceIndex(serviceType);

    // Detect circular dependencies
    {
      std::vector<std::type_index>::const_iterator duplicate = std::find(
        this->resolutionStack.begin(), this->resolutionStack.end(), serviceIndex
      );
      if(duplicate != this->resolutionStack.end()) [[unlikely]] {
        throw Errors::CyclicDependencyError(u8"Service dependency cycle detected");
      }
    }

    // Put the next service on the stack and try to resolve it. This guarantees that,
    // should a dependency cycle involving this service type happen, the cyclic
    // dependency error is detected.
    this->resolutionStack.emplace_back(serviceIndex);
    {
      ON_SCOPE_EXIT { this->resolutionStack.pop_back(); };
      //ActivateSingletonService(serviceIndex);
      // TODO: Implement StandardServiceProvider::ResolutionContext::TryGetService() method
      throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
    }
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
    (void)serviceType;
    // TODO: Implement StandardServiceProvider::ResolutionContext::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &StandardServiceProvider::ResolutionContext::ActivateSingletonService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // This is the service resolution context, meaning that the service provider already
    // needed to look up the first service and this is a sub-dependency. When this code
    // runs, the root service provider is currently holding the mutex lock. So we do not
    // need double-checked locking here and are allowed to modify the instances array.
    bool isAlreadyCreated = this->services.PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(this->mutexAcquired.load(std::memory_order::consume)) {
      if(!isAlreadyCreated) [[likely]] {
        if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
          new(this->services.Instances + uniqueServiceIndex) std::any(
            serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance)
          );
        } else {
          new(this->services.Instances + uniqueServiceIndex) std::any(
            std::move(serviceIterator->second.Factory(*this))
          );
        }

        this->services.PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);
      }

      return this->services.Instances[uniqueServiceIndex];
    } else { // ^^ mutex already held ^^ // vv mutex needs to be acquired vv
      this->services.ChangeMutex.lock();
      this->mutexAcquired.store(true, std::memory_order::release);
      return ActivateSingletonService(serviceIterator); // Re-enter -> double-checked locking
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::ActivateTransientService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
      return serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance);
    } else {
      return serviceIterator->second.Factory(*this);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::StandardServiceProvider(
    std::shared_ptr<StandardInstanceSet> &&services
  ) :
    emptyAny(),
    services(services) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::~StandardServiceProvider() = default;

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceProvider::CreateScope() {
    return std::make_shared<StandardServiceScope>(
      StandardInstanceSet::Create(
        this->services->Bindings, this->services->Bindings->ScopedServices
      ),
      this->services
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::TryGetService(const std::type_info &serviceType) {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        return this->emptyAny;
      } else {
        ResolutionContext deepServiceProvider(*this->services);
        return deepServiceProvider.ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(!isAlreadyCreated) [[unlikely]] {
      ResolutionContext deepServiceProvider(*this->services);
      deepServiceProvider.ActivateSingletonService(serviceIterator);
    }

    return this->services->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::GetService(const std::type_info &serviceType) {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        throwUnresolvedDependencyException(serviceTypeIndex);
      } else {
        ResolutionContext deepServiceProvider(*this->services);
        return deepServiceProvider.ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(!isAlreadyCreated) [[unlikely]] {
      ResolutionContext deepServiceProvider(*this->services);
      deepServiceProvider.ActivateSingletonService(serviceIterator);
    }

    return this->services->Instances[uniqueServiceIndex];
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

  void StandardServiceProvider::throwUnresolvedDependencyException(
    const std::type_index &serviceTypeIndex
  ) {
    // We could just output a plain "service not registered" message, but a common mistake
    // users may make is to try and depend on a scoped service from a singleton service,
    // so if the service type is indeed registered as a scoped service, print an alternative
    // exception message that may save the user some time and headaches :-)
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      serviceIterator = services->Bindings->ScopedServices.find(serviceTypeIndex)
    );
    if(serviceIterator == services->Bindings->ScopedServices.end()) [[likely]] {

      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceTypeIndex.name());
      message.append(serviceTypeIndex.name(), serviceTypeIndex.name() + length);
      message.append(u8"' (name may be mangled) has not been registered", 47);
      throw Errors::UnresolvedDependencyError(message);

    } else { // ^^ wholly unknown service ^^ // vv singleton depends on scoped vv

      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceTypeIndex.name());
      message.append(serviceTypeIndex.name(), serviceTypeIndex.name() + length);
      message.append(
        u8"' (name may be mangled) is a scoped service and cannot be requested from "
        u8"the root-level service provider",
        104
      );
      throw Errors::UnresolvedDependencyError(message);

    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
