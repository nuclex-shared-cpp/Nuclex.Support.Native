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

#include "./StandardServiceScope.h"

#include "Nuclex/Support/Errors/UnresolvedDependencyError.h"
#include "Nuclex/Support/Errors/CyclicDependencyError.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()

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

  /// <summary>
  ///   Throws an exception that indicates that the specified service could not be resovled
  /// </summary>
  /// <param nma=e"serviceTypeIndex">
  ///   The <code>std::type_index</code> of the service that could not be resolved
  /// </param>
  [[noreturn]] void throwUnresolvedDependencyException(
    const std::type_index &serviceTypeIndex
  ) {
    using Nuclex::Support::Errors::UnresolvedDependencyError;

    std::u8string message(u8"Requested service '", 19);
    std::string::size_type length = std::char_traits<char>::length(serviceTypeIndex.name());
    message.append(serviceTypeIndex.name(), serviceTypeIndex.name() + length);
    message.append(u8"' (name may be mangled) has not been registered", 47);
    throw UnresolvedDependencyError(message);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceScope::ResolutionContext::ResolutionContext(
    StandardInstanceSet &scopedInstanceSet,
    StandardInstanceSet &singletonInstanceSet
  ) :
    StandardServiceProvider::ResolutionContext(singletonInstanceSet),
    scopedServices(scopedInstanceSet),
    mutexAcquired(false),
    resolutionStack() {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceScope::ResolutionContext::~ResolutionContext() {
    if(this->mutexAcquired.load(std::memory_order::consume)) {
      this->scopedServices.ChangeMutex.unlock();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &StandardServiceScope::ResolutionContext::ActivateScopedService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    assert(this->mutexAcquired.load(std::memory_order::relaxed));
/*
    CheckForDependencyCycle(serviceIterator->first);
    this->resolutionStack.emplace_back(serviceIterator->first);
    ON_SCOPE_EXIT { this->resolutionStack.pop_back(); };

    // When this method is called, a requested service instance was not created yet,
    // so we definitely need to construct it now.
    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;
    {
      if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
        new(this->services.Instances + uniqueServiceIndex) std::any(
          serviceIterator->second.ProvidedInstance
        );
      } else {
        new(this->services.Instances + uniqueServiceIndex) std::any(
          std::move(serviceIterator->second.Factory(*this))
        );
      }

      this->services.PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);
    }

    return this->services.Instances[uniqueServiceIndex];
*/
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceScope::ResolutionContext::CreateScope() {
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
      reinterpret_cast<const char *>(u8"Cannot create scopes in a service resolution chain")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceScope::ResolutionContext::TryGetService(
    const std::type_info &serviceType
  ) {
    std::type_index serviceTypeIndex(serviceType);

    const StandardBindingSet &bindings = *this->scopedServices.Bindings;

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(bindings.ScopedServices, serviceTypeIndex)
    );
    if(serviceIterator == bindings.SingletonServices.end()) {
      serviceIterator = findLast(bindings.SingletonServices, serviceTypeIndex);
      if(serviceIterator == bindings.TransientServices.end()) [[unlikely]] {
        serviceIterator = findLast(bindings.TransientServices, serviceTypeIndex);
        if(serviceIterator == bindings.TransientServices.end()) [[unlikely]] {
          return std::any();
        } else {
          // TODO: Intering from here, we're not sure the singleton mutex is acquired
          return ActivateSingletonService(serviceIterator);
        }
      } else {
        return ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->scopedServices.PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->scopedServices.Instances[uniqueServiceIndex];
    } else {
      return ActivateScopedService(serviceIterator);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceScope::ResolutionContext::GetService(
    const std::type_info &serviceType
  ) {
    std::type_index serviceTypeIndex(serviceType);

    const StandardBindingSet &bindings = *this->scopedServices.Bindings;

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(bindings.ScopedServices, serviceTypeIndex)
    );
    if(serviceIterator == bindings.SingletonServices.end()) {
      serviceIterator = findLast(bindings.SingletonServices, serviceTypeIndex);
      if(serviceIterator == bindings.TransientServices.end()) [[unlikely]] {
        serviceIterator = findLast(bindings.TransientServices, serviceTypeIndex);
        if(serviceIterator == bindings.TransientServices.end()) [[unlikely]] {
          return std::any();
        } else {
          // TODO: Intering from here, we're not sure the singleton mutex is acquired
          return ActivateSingletonService(serviceIterator);
        }
      } else {
        return ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->scopedServices.PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->scopedServices.Instances[uniqueServiceIndex];
    } else {
      return ActivateScopedService(serviceIterator);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceScope::ResolutionContext::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    (void)serviceType;
    // TODO: Implement StandardServiceScope::ResolutionContext::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceScope::ResolutionContext::GetServices(
    const std::type_info &serviceType
  ) {
    (void)serviceType;
    // TODO: Implement StandardServiceScope::ResolutionContext::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceScope::StandardServiceScope(
    std::shared_ptr<StandardInstanceSet> &&scopedServices,
    const std::shared_ptr<StandardInstanceSet> &singletonServices
  ) :
    scopedServices(scopedServices),
    singletonServices(singletonServices) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceScope::~StandardServiceScope() = default;

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceScope::ResolutionContext::AcquireScopedChangeMutex() {
    if(!this->mutexAcquired.load(std::memory_order::consume)) [[likely]] {
      this->scopedServices.ChangeMutex.lock();
      this->mutexAcquired.store(true, std::memory_order::release);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceScope::CreateScope() {
    return std::make_shared<StandardServiceScope>(
      StandardInstanceSet::Create(
        this->scopedServices->Bindings, this->scopedServices->Bindings->ScopedServices
      ),
      this->singletonServices
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceScope::TryGetService(const std::type_info &serviceType) {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceScope::GetService(const std::type_info &serviceType) {
#if 0
    return locateBindingAndProvideServiceInstance<true>(
      this->scopedServices,
      this->singletonServices,
      serviceType,
      *this,
      &StandardServiceScope::fetchOrActivateScopedService,
      &StandardServiceScope::fetchOrActivateSingletonService
    );
#endif
    throw - 1;
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceScope::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    (void)serviceType;
    // TODO: Implement StandardServiceScope::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceScope::GetServices(
    const std::type_info &serviceType
  ) {
    (void)serviceType;
    // TODO: Implement StandardServiceScope::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
