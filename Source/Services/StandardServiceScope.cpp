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

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Alias for the <code>std::type_index</code> binding multi map</summary>
  typedef Nuclex::Support::Services::StandardBindingSet::TypeIndexBindingMultiMap (
    TypeIndexBindingMultiMap
  );

  // ------------------------------------------------------------------------------------------- //
#if 0 // Nope, this is becoming to confuddled, we'll use chained StandardInstanceSets
  /// <summary>
  ///   Locates the services binding for the requested service and provides an instance of
  ///   the service by either fetching it from the service container or activating it
  /// </summary>
  /// <typeparam name="ThrowIfMissing">
  ///   Whether to throw an exception if the service has not been bound
  /// </typeparam>
  /// <typeparam name="TProvider">
  ///   Service provider on which the service fetch/activaion method is being called
  /// </typeparam>
  /// <param name="services">Container in which the service instance will be stored</param>
  /// <param name="serviceType">
  ///   Type of service of which an instance needs to be provided
  /// </param>
  /// <param name="provider">Provider on which the activation method will be called</param>
  /// <param name="fetchOrActiveService">
  ///   Method that performs the actual check looking for an existing service in the service
  ///   container and which created an instance of the service if needed
  /// </param>
  /// <returns>The requested service instance wrapped in an <code>std::any</code></returns>
  template<bool ThrowOnMissing, typename TProvider>
  std::any locateBindingAndProvideServiceInstance(
    const std::shared_ptr<Nuclex::Support::Services::StandardInstanceSet> &scopedServices,
    const std::shared_ptr<Nuclex::Support::Services::StandardInstanceSet> &singletonServices,
    const std::type_info &serviceType,
    TProvider &provider,
    std::any (TProvider::*fetchOrActivateScopedService)(
      const TypeIndexBindingMultiMap::const_iterator &
    ),
    std::any (TProvider::*fetchOrActivateSingletonService)(
      const TypeIndexBindingMultiMap::const_iterator &
    )
  ) {
    using Nuclex::Support::Services::StandardBindingSet;
    using Nuclex::Support::Errors::UnresolvedDependencyError;
    const std::type_index serviceIndex(serviceType);

    // First, look for a scoped service that we can deliver. For the same reason we
    // look for singletons first in the global service provider, for performance reasons
    // we expect a scoped service to be the most likely request and check for it first.
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      scopedServices->OwnBindings.find(serviceIndex)
    );
    if(serviceIterator != scopedServices->OwnBindings.end()) [[likely]] {
      return (provider.*fetchOrActivateScopedService)(serviceIterator);
    }

    // Then try to find a singleton service. This sould be the second most likely
    // service lifetime for which instances will be requested, though you can argue
    // that transient services may be equally likely.
    serviceIterator = singletonServices->OwnBindings.find(serviceIndex);
    if(serviceIterator != singletonServices->OwnBindings.end()) [[likely]] {
      return (provider.*fetchOrActivateSingletonService)(serviceIterator);
    }

    // It was not a registered scoped or singleton service. So next, we'll check if
    // it is a transient service. These services can be requested at any level.
    serviceIterator = scopedServices->Bindings->TransientServices.find(serviceIndex);
    if(serviceIterator != scopedServices->Bindings->TransientServices.end()) [[likely]] {
      return serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance);
    }

    // At this point, we know the service cannot be provided by the root level service
    // provider. However, as a small courtesy to the user, we'll look in the scoped services
    // to provide a helpful error message if it appears that the user tried to request
    // a scoped services from the root level service provider.
    if constexpr(ThrowOnMissing) {
      std::u8string message(u8"Requested service '", 19);
      std::string::size_type length = std::char_traits<char>::length(serviceType.name());
      message.append(serviceType.name(), serviceType.name() + length);
      message.append(u8"' (name may be mangled) has not been registered", 47);
      throw UnresolvedDependencyError(message);
    } else {
      return std::any();
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services {

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
    (void)serviceType;
    // TODO: Implement StandardServiceScope::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
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
