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

    #pragma region class ResolutionContext

    /// <summary>Proxy that Handles service resolution and </summary>
    public: class ResolutionContext : public ServiceProvider {

      /// <summary>
      ///   Initializes a new service provider providing the specified set of services
      /// </summary>
      /// <param name="instanceSet">
      ///   Instance set to check for existing service instances and in which created
      ///   services will be placed.
      /// </param>
      /// <param name="outerServiceType">
      ///   Initial service type that started the dependency resolution chain. Provided
      ///   so it's on record, too, for the dependency cycle detection code.
      /// </param>
      public: explicit ResolutionContext(
        StandardInstanceSet &instanceSet,
        const std::type_index &outerServiceType
      );

      /// <summary>Destroys the service provider and frees all resources</summary>
      public: ~ResolutionContext() override;

      /// <summary>Fetches an already activated singleton service or activates it</summary>
      /// <param name="serviceIterator">
      ///   Iterator to the requested service in the singleton service bindings
      /// </param>
      /// <returns>An <code>std::any</code> that contains the service instance</returns>
      public: std::any FetchOrActivateSingletonService(
        const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
      );

      /// <summary>Creates a new service scope</summary>
      /// <returns>The new service scope</returns>
      public: std::shared_ptr<ServiceScope> CreateScope() override;

      /// <summary>Tries to provide the specified service</summary>
      /// <param name="serviceType">Type of service that will be provided</param>
      /// <returns>An <code>std::any</code> containing the service if it can be provided</returns>
      protected: std::any TryGetService(const std::type_info &typeInfo) override;

      /// <summary>Provides the specified service</summary>
      /// <param name="serviceType">Type of service that will be provided</param>
      /// <returns>An <code>std::any</code> containing the service</returns>
      protected: std::any GetService(const std::type_info &typeInfo) override;

      /// <summary>Provides a factory method that creates the specified service</summary>
      /// <param name="serviceType">Type of service that will be provided</param>
      /// <returns>
      ///   A factory method that will provide an instance of the specified service
      ///   as a <code>std::shared_ptr</code> wrapped in an <code>std::any</code>.
      /// </returns>
      protected: std::function<std::any()> GetServiceFactory(
        const std::type_info &typeInfo
      ) const override;

      /// <summary>Provides all instances registered for the specified service</summary>
      /// <param name="serviceType">Type of service that will be provided</param>
      /// <returns>A list of <code>std::any</code>s containing each service</returns>
      protected: std::vector<std::any> GetServices(const std::type_info &typeInfo) override;

      /// <summary>Container for the instances of all singleton services</summary>
      private: StandardInstanceSet &services;
      /// <summary>Stack of services currently being resolved</summary>
      private: std::vector<std::type_index> resolutionStack;

    };

    #pragma endregion // class ResolutionContext

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

    // TODO: Add a TryFetchOrCreateScopedServiceInstance() overload with chaining
    //   It should get another parameter which is another instance set it will fall
    //   back to if this instance set cannot provide the service from the scoped service
    //   bindings. It is important to use the other 'StandardInstanceSet' for that because
    //   the singleton services live in a whole other 'StandardInstancerSet' with its
    //   own mutex that needs to go through the correct locking procedure, too.

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
