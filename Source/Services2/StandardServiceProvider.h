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

#ifndef NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICEPROVIDER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services2/ServiceProvider.h"

namespace Nuclex::Support::Services2 {
  class StandardBindingSet;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Implementation of the service provider that's built by
  ///   the <see cref="StandardServiceCollection" />
  /// </summary>
  class StandardServiceProvider : public ServiceProvider {

    /// <summary>
    ///   Initializes a new service provider providing the specified set of services
    /// </summary>
    /// <param name="bindings">
    ///   Binding of all service that will be made available through the service provider
    /// </param>
    public: StandardServiceProvider(
      const std::shared_ptr<StandardBindingSet> &bindings
    );

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
    protected: std::function<std::any> GetServiceFactory(
      const std::type_info &typeInfo
    ) const override;

    /// <summary>Provides all instances registered for the specified service</summary>
    /// <param name="serviceType">Type of service that will be provided</param>
    /// <returns>A list of <code>std::any</code>s containing each service</returns>
    protected: std::vector<std::any> GetServices(const std::type_info &typeInfo) override;

    /// <summary>Service bindings the provider is offering</summary>
    private: std::shared_ptr<StandardBindingSet> bindings;
    /// <summary>Instances of all singleton services</summary>
    private: std::shared_ptr<std::any[]> singletonInstances;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICEPROVIDER_H
