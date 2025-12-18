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

#ifndef NUCLEX_SUPPORT_SERVICES_STANDARDSERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES_STANDARDSERVICEPROVIDER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceProvider.h"

#include "./StandardInstanceSet.h"

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Implementation of the service provider that's built by
  ///   the <see cref="StandardServiceCollection" />
  /// </summary>
  class StandardServiceProvider : public ServiceProvider {

    #pragma region class ResolutionContext

    /// <summary>Proxy that handles service resolution of singleton services</summary>
    public: class ResolutionContext : public ServiceProvider {

      /// <summary>
      ///   Initializes a new service provider providing the specified set of services
      /// </summary>
      /// <param name="instanceSet">
      ///   Instance set to check for existing service instances and in which created
      ///   services will be placed.
      /// </param>
      public: explicit ResolutionContext(
        const std::shared_ptr<StandardInstanceSet> &instanceSet
      );

      /// <summary>Destroys the service provider and frees all resources</summary>
      public: ~ResolutionContext() override;

      /// <summary>Acquires the mutex required to alter the singleton services</summary>
      public: void AcquireSingletonChangeMutex();

      /// <summary>Fetches an already activated singleton service or activates it</summary>
      /// <param name="serviceIterator">
      ///   Iterator to the requested service in the singleton service bindings
      /// </param>
      /// <returns>An <code>std::any</code> that contains the service instance</returns>
      public: const std::any &ActivateSingletonService(
        const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
      );

      /// <summary>Creates a new instance of a transient service</summary>
      /// <param name="serviceIterator">
      ///   Iterator to the requested service in the singleton service bindings
      /// </param>
      /// <returns>An <code>std::any</code> that contains the service instance</returns>
      public: std::any ActivateTransientService(
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

      /// <summary>Accesses the resolution stack that is used to prevent cycles</summary>
      /// <returns>A stack containing the trail of the requested dependencies</returns>
      protected: std::vector<std::type_index> &GetResolutionStack();

      /// <summary>Throws an exception if a dependency cycle is detected</summary>
      /// <param name="serviceTypeIndex">Service that is going to be resolved</param>
      protected: void CheckForDependencyCycle(const std::type_index &serviceTypeIndex);

      /// <summary>Container for the instances of all singleton services</summary>
      protected: const std::shared_ptr<StandardInstanceSet> &services;
      /// <summary>Whether the context has acquired the service state update mutex</summary>
      private: std::atomic<bool> mutexAcquired;
      /// <summary>Stack of services currently being resolved</summary>
      private: std::vector<std::type_index> resolutionStack;

    };

    #pragma endregion // class ResolutionContext

    /// <summary>
    ///   Initializes a new service provider providing the specified set of services
    /// </summary>
    /// <param name="services">
    ///   Bindings and instances of all services that will be made available
    /// </param>
    public: StandardServiceProvider(std::shared_ptr<StandardInstanceSet> &&services);

    /// <summary>Destroys the service provider and frees all resources</summary>
    public: ~StandardServiceProvider() override;

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
    /// <summary>Service bindings and instances the provider is offering</summary>
    private: std::shared_ptr<StandardInstanceSet> services;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDSERVICEPROVIDER_H
