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

    /// <summary>Creates or fetches an instance of the specified service</summary>
    /// <param name="serviceProvider">
    ///   Service provider that will be forwarded to the service factory if a new service
    ///   needs to be constructed.
    /// </param>
    /// <param name="service">
    ///   Service (an iterator in the OwnBindings multimap) whose instance will be created
    ///   or returned if it already exists
    /// </param>
    /// <returns>The `std::any` that contains the service instance</returns>
    public: const std::any &CreateOrFetchServiceInstance(
      ServiceProvider &serviceProvider,
      const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &service
    );


    // TODO: Remove 'serviceProvider' from the CreateOrFetchServiceInstance method.
    //   Since we're now providing an internal service provider that is necessary to
    //   catch cyclic dependencies, we can as well avoid even exposing the outer
    //   service provider to the factory methods. Move the ResolutionContext from
    //   the StandardServiceProvider here.

    // TODO: Add another CreateOrFetchServiceInstance overload with chaining
    //   It should get another parameter which is another instance set it will fall
    //   back to if the original instance set cannot provide the service

    // TODO: Return 'this->emptyAny' instead of throwing an exception on unknown services
    //   This will allow the caller (either the root service provider or a service scope)
    //   to decide whether to return the empty std::any (for 'TryGetService()') or whether
    //   to throw an exception which can be refined by checking for wrong lifetime issues.

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
    /// <summary>An std::any instance that contains nothing</summary>
    private: const std::any emptyAny;

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
