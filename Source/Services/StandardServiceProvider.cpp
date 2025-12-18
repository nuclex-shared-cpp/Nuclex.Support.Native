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
  /// <param name="services">
  ///   Service bindings, used to check for a scoped service that, when present, results in
  ///   a slightly altered error message to clue the user on a possible wrong lifetime.
  /// </param>
  /// <param nmae="serviceTypeIndex">
  ///   The <code>std::type_index</code> of the service that could not be resolved
  /// </param>
  [[noreturn]] void throwUnresolvedDependencyException(
    const Nuclex::Support::Services::StandardInstanceSet &services,
    const std::type_index &serviceTypeIndex
  ) {
    using Nuclex::Support::Services::StandardBindingSet;
    using Nuclex::Support::Errors::UnresolvedDependencyError;

    // We could just output a plain "service not registered" message, but a common mistake
    // users may make is to try and depend on a scoped service from a singleton service,
    // so if the service type is indeed registered as a scoped service, print an alternative
    // exception message that may save the user some time and headaches :-)
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      services.Bindings->ScopedServices.find(serviceTypeIndex)
    );
    if(serviceIterator == services.Bindings->ScopedServices.end()) [[likely]] {

      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceTypeIndex.name());
      message.append(serviceTypeIndex.name(), serviceTypeIndex.name() + length);
      message.append(u8"' (name may be mangled) has not been registered", 47);
      throw UnresolvedDependencyError(message);

    } else { // ^^ wholly unknown service ^^ // vv singleton depends on scoped vv

      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceTypeIndex.name());
      message.append(serviceTypeIndex.name(), serviceTypeIndex.name() + length);
      message.append(
        u8"' (name may be mangled) is a scoped service and cannot be requested from "
        u8"the root-level service provider",
        104
      );
      throw UnresolvedDependencyError(message);

    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::ResolutionContext(
    const std::shared_ptr<StandardInstanceSet> &services
  ) :
    services(services),
    mutexAcquired(false),
    resolutionStack() {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::ResolutionContext::~ResolutionContext() {
    if(this->mutexAcquired.load(std::memory_order::consume)) {
      this->services->ChangeMutex.unlock();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceProvider::ResolutionContext::AcquireSingletonChangeMutex() {
    if(!this->mutexAcquired.load(std::memory_order::consume)) [[likely]] {
      this->services->ChangeMutex.lock();
      this->mutexAcquired.store(true, std::memory_order::release);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &StandardServiceProvider::ResolutionContext::ActivateSingletonService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    assert(this->mutexAcquired.load(std::memory_order::relaxed));

    CheckForDependencyCycle(serviceIterator->first);
    this->resolutionStack.emplace_back(serviceIterator->first);
    ON_SCOPE_EXIT { this->resolutionStack.pop_back(); };

    // When this method is called, a requested service instance was not created yet,
    // so we definitely need to construct it now.
    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;
    {
      if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
        new(this->services->Instances + uniqueServiceIndex) std::any(
          serviceIterator->second.ProvidedInstance
        );
      } else {
        new(this->services->Instances + uniqueServiceIndex) std::any(
          std::move(serviceIterator->second.Factory(*this))
        );
      }

      this->services->PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);
    }

    return this->services->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::ActivateTransientService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    CheckForDependencyCycle(serviceIterator->first);
    this->resolutionStack.emplace_back(serviceIterator->first);
    ON_SCOPE_EXIT { this->resolutionStack.pop_back(); };

    if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
      return serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance);
    } else {
      return serviceIterator->second.Factory(*this);
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
      reinterpret_cast<const char *>(u8"Cannot create scopes in a service resolution chain")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::TryGetService(
    const std::type_info &serviceType
  ) {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        return std::any();
      } else {
        return ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->services->Instances[uniqueServiceIndex];
    } else {
      return ActivateSingletonService(serviceIterator);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::ResolutionContext::GetService(
    const std::type_info &serviceType
  ) {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        throwUnresolvedDependencyException(*this->services, serviceTypeIndex);
      } else {
        return ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->services->Instances[uniqueServiceIndex];
    } else {
      return ActivateSingletonService(serviceIterator);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceProvider::ResolutionContext::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implementation registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        throwUnresolvedDependencyException(*this->services, serviceTypeIndex);
      } else {
        std::shared_ptr<StandardInstanceSet> services = this->services;
        return [services, serviceIterator]() -> std::any {
          ResolutionContext deepServiceProvider(services);
          return deepServiceProvider.ActivateTransientService(serviceIterator);
        };
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;
    std::shared_ptr<StandardInstanceSet> services = this->services;
    return [services, serviceIterator, uniqueServiceIndex]() -> std::any {
      bool isAlreadyCreated = services->PresenceFlags[uniqueServiceIndex].load(
        std::memory_order::consume
      );
      if(isAlreadyCreated) [[likely]] {
        return services->Instances[uniqueServiceIndex];
      } else {
        ResolutionContext deepServiceProvider(services);
        deepServiceProvider.AcquireSingletonChangeMutex();
        return deepServiceProvider.ActivateSingletonService(serviceIterator);
      }
    };
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceProvider::ResolutionContext::GetServices(
    const std::type_info &serviceType
  ) {
    std::vector<std::any> result;
    std::type_index serviceTypeIndex(serviceType);

    // Look for the first service implemented registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      this->services->Bindings->SingletonServices.find(serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = this->services->Bindings->TransientServices.find(serviceTypeIndex);
      if(serviceIterator != this->services->Bindings->TransientServices.end()) [[unlikely]] {
        do {
          result.push_back(ActivateTransientService(serviceIterator));
          ++serviceIterator;
          if(serviceIterator == this->services->Bindings->TransientServices.end()) {
            break;
          }
        } while(serviceIterator->first == serviceTypeIndex);
      }
    } else {
      do {
        std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

        // Check, without locking, if the instance has already been created. If so,
        // there's no need to enter the mutex since we're not modifying our state.
        bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
          std::memory_order::consume
        );
        if(isAlreadyCreated) [[likely]] {
          result.push_back(this->services->Instances[uniqueServiceIndex]);
        } else {
          result.push_back(ActivateSingletonService(serviceIterator));
        }

        ++serviceIterator;
        if(serviceIterator == this->services->Bindings->SingletonServices.end()) {
          break;
        }
      } while(serviceIterator->first == serviceTypeIndex);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::type_index> &StandardServiceProvider::ResolutionContext::GetResolutionStack() {
    return this->resolutionStack;
  }

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceProvider::ResolutionContext::CheckForDependencyCycle(
    const std::type_index &serviceTypeIndex
  ) {
    std::vector<std::type_index>::const_iterator duplicate = std::find(
      this->resolutionStack.begin(), this->resolutionStack.end(), serviceTypeIndex
    );
    if(duplicate != this->resolutionStack.end()) [[unlikely]] {
      throw Errors::CyclicDependencyError(u8"Service dependency cycle detected");
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

    // The following code block is almost identical to ResolutionContext::TryGetService().
    // This serves two purposes:
    //   1) we want the early check to be very quick, directly from the calling application
    //      via a single vtable-call to the full check.
    //   2) we need to create the resolution context here. It is what prevents cyclic
    //      dependencies from getting into a stack overlow.

    // Look for the last service implementation registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        return this->emptyAny; // Accept that the service has not been bound
      } else {
        ResolutionContext deepServiceProvider(this->services);
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
      ResolutionContext deepServiceProvider(this->services);
      deepServiceProvider.AcquireSingletonChangeMutex();
      deepServiceProvider.ActivateSingletonService(serviceIterator);
    }

    return this->services->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::GetService(const std::type_info &serviceType) {
    std::type_index serviceTypeIndex(serviceType);

    // The following code block is almost identical to ResolutionContext::GetService().
    // This serves two purposes:
    //   1) we want the early check to be very quick, directly from the calling application
    //      via a single vtable-call to the full check.
    //   2) we need to create the resolution context here. It is what prevents cyclic
    //      dependencies from getting into a stack overlow.
    //

    // Look for the last service implementation registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        throwUnresolvedDependencyException(*this->services, serviceTypeIndex);
      } else {
        ResolutionContext deepServiceProvider(this->services);
        return deepServiceProvider.ActivateTransientService(serviceIterator);
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->services->Instances[uniqueServiceIndex];
    } else {
      ResolutionContext deepServiceProvider(this->services);
      deepServiceProvider.AcquireSingletonChangeMutex();
      return deepServiceProvider.ActivateSingletonService(serviceIterator);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceProvider::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    std::type_index serviceTypeIndex(serviceType);

    // Look for the last service implementation registered for the requested service type
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      findLast(this->services->Bindings->SingletonServices, serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = findLast(this->services->Bindings->TransientServices, serviceTypeIndex);
      if(serviceIterator == this->services->Bindings->TransientServices.end()) [[unlikely]] {
        throwUnresolvedDependencyException(*this->services, serviceTypeIndex);
      } else {
        std::shared_ptr<StandardInstanceSet> services = this->services;
        return [services, serviceIterator]() -> std::any {
          ResolutionContext deepServiceProvider(services);
          return deepServiceProvider.ActivateTransientService(serviceIterator);
        };
      }
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;
    std::shared_ptr<StandardInstanceSet> services = this->services;
    return [services, serviceIterator, uniqueServiceIndex]() -> std::any {
      bool isAlreadyCreated = services->PresenceFlags[uniqueServiceIndex].load(
        std::memory_order::consume
      );
      if(isAlreadyCreated) [[likely]] {
        return services->Instances[uniqueServiceIndex];
      } else {
        ResolutionContext deepServiceProvider(services);
        deepServiceProvider.AcquireSingletonChangeMutex();
        return deepServiceProvider.ActivateSingletonService(serviceIterator);
      }
    };
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceProvider::GetServices(const std::type_info &serviceType) {
    std::vector<std::any> result;
    std::type_index serviceTypeIndex(serviceType);

    // First, check the singleton services for the requested service type. A service can
    // only be in one lifetime, so once we find the service, we just need to take all
    // instances in the scope we find the first one in.
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      this->services->Bindings->SingletonServices.find(serviceTypeIndex)
    );
    if(serviceIterator == this->services->Bindings->SingletonServices.end()) [[unlikely]] {
      serviceIterator = this->services->Bindings->TransientServices.find(serviceTypeIndex);
      if(serviceIterator != this->services->Bindings->TransientServices.end()) [[unlikely]] {
        do {
          ResolutionContext deepServiceProvider(this->services);
          result.push_back(deepServiceProvider.ActivateTransientService(serviceIterator));

          ++serviceIterator;
          if(serviceIterator == this->services->Bindings->TransientServices.end()) {
            break;
          }
        } while(serviceIterator->first == serviceTypeIndex);
      }
    } else {
      do {
        std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

        // Check, without locking, if the instance has already been created. If so,
        // there's no need to enter the mutex since we're not modifying our state.
        bool isAlreadyCreated = this->services->PresenceFlags[uniqueServiceIndex].load(
          std::memory_order::consume
        );
        if(isAlreadyCreated) [[likely]] {
          result.push_back(this->services->Instances[uniqueServiceIndex]);
        } else {
          ResolutionContext deepServiceProvider(this->services);
          deepServiceProvider.AcquireSingletonChangeMutex();
          result.push_back(deepServiceProvider.ActivateSingletonService(serviceIterator));
        }

        ++serviceIterator;
        if(serviceIterator == this->services->Bindings->SingletonServices.end()) {
          break;
        }
      } while(serviceIterator->first == serviceTypeIndex);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
