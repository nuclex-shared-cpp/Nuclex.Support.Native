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

    /// <summary>
    ///   Proxy that handles service resolution of scoped and singleton services
    /// </summary>
    public: class ResolutionContext : public StandardServiceProvider::ResolutionContext {

      /// <summary>
      ///   Initializes a new service provider providing the specified set of services
      /// </summary>
      /// <param name="scopedInstanceSet">
      ///   Instance set to check for existing scoped service instances and in which
      ///   created scoped services will be placed.
      /// </param>
      /// <param name="singletonInstanceSet">
      ///   Instance set to check for existing singleton service instances and in which
      ///   created singelton services will be placed.
      /// </param>
      public: explicit ResolutionContext(
        const std::shared_ptr<StandardInstanceSet> &scopedInstanceSet,
        const std::shared_ptr<StandardInstanceSet> &singletonInstanceSet
      );

      /// <summary>Destroys the service provider and frees all resources</summary>
      public: ~ResolutionContext() override;

      /// <summary>Acquires the mutex required to alter the scoped services</summary>
      public: void AcquireScopedChangeMutex();

      /// <summary>Fetches an already activated scoped service or activates it</summary>
      /// <param name="serviceIterator">
      ///   Iterator to the requested service in the scoped service bindings
      /// </param>
      /// <returns>An <code>std::any</code> that contains the service instance</returns>
      public: const std::any &ActivateScopedService(
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
      private: const std::shared_ptr<StandardInstanceSet> &scopedServices;
      /// <summary>Whether the context has acquired the service state update mutex</summary>
      private: std::atomic<bool> mutexAcquired;
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

    /// <summary>An <code>std::any</code> instance that stays empty</summary>
    private: const std::any emptyAny;
    /// <summary>Service bindings and instances the scope is offering</summary>
    private: std::shared_ptr<StandardInstanceSet> scopedServices;
    /// <summary>Service bindings and instances the global provider is offering</summary>
    private: std::shared_ptr<StandardInstanceSet> singletonServices;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDSERVICESCOPE_H
