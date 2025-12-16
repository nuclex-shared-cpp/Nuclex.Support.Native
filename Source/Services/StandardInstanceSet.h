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

#ifndef NUCLEX_SUPPORT_SERVICES_STANDARDINSTANCESET_H
#define NUCLEX_SUPPORT_SERVICES_STANDARDINSTANCESET_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceProvider.h"

#include "./StandardBindingSet.h"

#include <cstddef> // for std::size_t
#include <atomic> // for std::atomic<>
#include <mutex> // for std::mutex
#include <typeindex> // for std::type_index
#include <optional> // for std::optional

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores instances of created services for a service provider</summary>
  class StandardInstanceSet {

    /// <summary>Creates a service instance set for the specified binding subset</summary>
    /// <param name="bindings">Service bindings for which instances will be stored</param>
    /// <param name="ownBindings">Binding subset to allocate instances for</param>
    /// <returns>A new service instance set</returns>
    public: static std::shared_ptr<StandardInstanceSet> Create(
      const std::shared_ptr<const StandardBindingSet> &bindings,
      const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
    );

    /// <summary>Initializes a new standard instance set</summary>
    /// <param name="bindings">Service bindings for which instances will be stored</param>
    /// <param name="ownBindings">Binding subset to allocate instances for</param>
    /// <remarks>
    ///   This constructor only works when called through Create() because it assumes that
    ///   all memory is allocated as one large block. Using this constructor normally will
    ///   cause UB, most likely a segmentation fault.
    /// </remarks>
    public: StandardInstanceSet(
      const std::shared_ptr<const StandardBindingSet> &bindings,
      const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
    );

    /// <summary>Frees all memory owned by the instance</summary>
    public: ~StandardInstanceSet();

#if 0
    /// <summary>Fetches or creates an instance of the specified service</summary>
    /// <param name="serviceType">Type of service of which an instance will be provided</param>
    /// <returns>
    ///   An <code>std::any</code> that either contains the service instance or is empty.
    ///   Only if the service is not bound will an empty <code>std::any</code> be returned,
    ///   any other problems (such as cyclic dependencies) still results in an exception
    /// </returns>
    public: const std::any &TryFetchOrCreateSingletonServiceInstance(
      const std::type_index &serviceTypeIndex
    );

    /// <summary>
    ///   Fetches or creates a scoped service instance or falls back to another set
    /// </summary>
    /// <param name="serviceTypeIndex">
    ///   Type of service of which an instance will be provided
    /// </param>
    /// <param name="fallback">
    ///   Instance set that will be queried if no scoped binding is available
    /// </param>
    /// <returns>
    ///   An <code>std::any</code> containing the service instance if available, otherwise
    ///   the empty instance placeholder
    /// </returns>
    public: const std::any &TryFetchOrCreateScopedServiceInstance(
      const std::type_index &serviceTypeIndex,
      StandardInstanceSet &fallback
    );
#endif
    
    /// <summary>Service bindings for which instances are being stored</summary>
    /// <remarks>
    ///   This must be stored for the <code>OwnBindings</code> attribute, which points
    ///   to either the singleton, scoped or transient bindings in this instane, remains
    ///   valid and doesn't become a dangling reference. It is also used for service
    ///   activation, of course.
    /// </remarks>
    public: std::shared_ptr<const StandardBindingSet> Bindings;
    /// <summary>
    ///   Bindings for which instances are managed (references either singleton bindings
    ///   or scoped bindings inside the referenced standard binding set).
    /// </summary>
    public: const StandardBindingSet::TypeIndexBindingMultiMap &OwnBindings;

    /// <summary>Mutex that must be held when updating an instance</summary>
    public: std::mutex ChangeMutex;
    /// <summary>Flag for each service that indicates whether it is present</summary>
    public: std::atomic<bool> *PresenceFlags;
    /// <summary>Instances of all services</summary>
    public: std::any *Instances;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDINSTANCESET_H
