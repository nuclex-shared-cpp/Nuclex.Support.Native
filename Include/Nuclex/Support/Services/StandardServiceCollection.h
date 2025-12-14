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

#ifndef NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_H
#define NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Services/ServiceCollection.h" // for ServiceCollection

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Standard implementation of the <see cref="ServiceCollection" /> that is used to
  ///   set up service bindings before constructing the actual service provider
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     The usage pattern of this dependency injector is to a) create a new
  ///     <code>ServiceCollection</code> (it can be temporary), then b) register all
  ///     services your application will need (typically done during start up, you can
  ///     split registration into components, i.e. <code>registerDatabaseServices()</code>),
  ///     then c) call <code>BuildServiceProvider()</code> to build the actual dependency
  ///     injector which will provide and own the service instances.
  ///   </para>
  ///   <para>
  ///     This pattern is common in more modern Java and .NET injectors from which this
  ///     dependency injection system took more than a little inspiration.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE StandardServiceCollection : public ServiceCollection {

    /// <summary>Initializes a new, empty service collection</summary>
    public: NUCLEX_SUPPORT_API StandardServiceCollection();
    /// <summary>Frees all resources owned by the service collection</summary>
    public: NUCLEX_SUPPORT_API ~StandardServiceCollection() override;

    // TODO: Implement a VerifyBindings() method to check consistency/completeness
    //   I believe that somehow, Boost's "callable traits" module is able to extract
    //   the argument types of a method (or constructor) in a way that makes them available
    //   on a way that they could be recorded and enable runtime checking (that's all
    //   we really want) of the service bindings that have been set up.

    /// <summary>Uses the services registered so far to build a service provider</summary>
    /// <returns>The new service provider</returns>
    public: NUCLEX_SUPPORT_API std::shared_ptr<
      ServiceProvider
    > BuildServiceProvider() const override;

    /// <summary>Removes all service bindings for the specified type</summary>
    /// <param name="serviceType">Type of service whose bindings will be removed</param>
    /// <returns>The number of removed service bindings for the service type</returns>
    protected: NUCLEX_SUPPORT_API std::size_t RemoveAll(
      const std::type_info &serviceType
    ) override;

    /// <summary>Adds the specified service binding to the collection</summary>
    /// <param name="serviceType">Type of service that will be bound</param>
    /// <param name="factoryMethod">
    ///   Method through which instances of the service will be created
    /// </param>
    /// <param name="lifetime">
    ///   Which lifetime category the service will use: singleton, scoped or transient
    /// </param>
    protected: NUCLEX_SUPPORT_API void AddServiceBinding(
      const std::type_info &serviceType,
      const std::function<std::any(ServiceProvider &)> &factoryMethod,
      ServiceLifetime lifetime
    ) override;

    /// <summary>Adds a binding for a transient service that clones a prototype</summary>
    /// <param name="serviceType">Type of service that will be bound</param>
    /// <param name="instance">
    ///   Existing instance that will be provided when the service is requested
    /// </param>
    /// <param name="cloneMethod">Method that will clone the existing instance</param>
    /// <param name="lifetime">
    ///   Which lifetime category the service will use: singleton, scoped or transient
    /// </param>
    protected: NUCLEX_SUPPORT_API void AddPrototypedService(
      const std::type_info &serviceType,
      const std::any &instance,
      const std::function<std::any(const std::any &)> &cloneMethod,
      ServiceLifetime lifetime
    ) override;

    // Private implementation details (the standard pImpl pattern)
    private: class PrivateImplementation;

    /// <summary>Holds the private implementation details of the service collection</summary>
    private: std::unique_ptr<PrivateImplementation> privateImplementation;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_H
