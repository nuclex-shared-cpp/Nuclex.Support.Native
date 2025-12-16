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

#ifndef NUCLEX_SUPPORT_SERVICES_STANDARDSERVICESCOPE_H
#define NUCLEX_SUPPORT_SERVICES_STANDARDSERVICESCOPE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceScope.h"

#include "./StandardServiceProvider.h"
//#include "./StandardInstanceSet.h"

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Nested service provider that can created scoped services</summary>
  class StandardServiceScope : public ServiceScope {

    #pragma region class ResolutionContext

    /// <summary>Proxy that Handles service resolution and </summary>
    public: class ResolutionContext : public StandardServiceProvider::ResolutionContext {

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
        StandardInstanceSet &instanceSet
      );

      /// <summary>Destroys the service provider and frees all resources</summary>
      public: ~ResolutionContext() override;

      /// <summary>Fetches an already activated singleton service or activates it</summary>
      /// <param name="serviceIterator">
      ///   Iterator to the requested service in the singleton service bindings
      /// </param>
      /// <returns>An <code>std::any</code> that contains the service instance</returns>
      public: const std::any &ActivateSingletonService(
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

    /// <summary>
    ///   Initializes a new service provider providing the specified set of services
    /// </summary>
    /// <param name="scopedServices">
    ///   Bindings and instances of all services whose lifetime is bound to the scope
    /// </param>
    /// <param name="singletonServices">
    ///   Bindings and instances of all services of the global service provider
    /// </param>
    public: StandardServiceScope(
      std::shared_ptr<StandardInstanceSet> &&scopedServices,
      const std::shared_ptr<StandardInstanceSet> &singletonServices
    );

    /// <summary>Destroys the service provider and frees all resources</summary>
    public: ~StandardServiceScope() override;

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
#if 0
    /// <summary>Fetches an already activated scoped service or activates it</summary>
    /// <param name="serviceIterator">
    ///   Iterator to the requested service in the scoped service bindings
    /// </param>
    /// <returns>An <code>std::any</code> that contains the service instance</returns>
    private: std::any fetchOrActivateScopedService(
      const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
    );

    /// <summary>Fetches an already activated singleton service or activates it</summary>
    /// <param name="serviceIterator">
    ///   Iterator to the requested service in the singleton service bindings
    /// </param>
    /// <returns>An <code>std::any</code> that contains the service instance</returns>
    private: std::any fetchOrActivateSingletonService(
      const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
    );
#endif
    /// <summary>Service bindings and instances the scope is offering</summary>
    private: std::shared_ptr<StandardInstanceSet> scopedServices;
    /// <summary>Service bindings and instances the global provider is offering</summary>
    private: std::shared_ptr<StandardInstanceSet> singletonServices;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDSERVICESCOPE_H
