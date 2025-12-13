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

#ifndef NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services2/ServiceScopeFactory.h"

#include <any> // for std::any, std::any_cast<>
#include <vector> // for std::vector<>
#include <functional> // for std::function<>

namespace Nuclex::Support::Services2 {
  class ServiceProvider;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides services to the application</summary>
  /// <remarks>
  ///   This is an interface through which services can be looked up. It is either used
  ///   manually (but beware of the service locator anti-pattern!) or as part of
  ///   a dependency injection framework.
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ServiceProvider : public ServiceScopeFactory {

    /// <summary>Destroys the service provider and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~ServiceProvider();

    /// <summary>Tries to provide the specified service</summary>
    /// <typeparam name="TService">Type of service that will be provided</typeparam>
    /// <returns>An <code>std::shared_ptr</code> for the service if it can be provided</returns>
    /// <remarks>
    ///   <para>
    ///     An empty <code>std::shared_ptr</code> will be returned if the specified service
    ///     has not been registered to the service provider and thus, it can neither provide
    ///     an existing instance nor construct a new one.
    ///   </para>
    ///   <para>
    ///     If there is another problem, this method will still throw an exception.
    ///   </para>
    /// </remarks>
    public: template<typename TService>
    inline std::shared_ptr<TService> TryGetService();

    /// <summary>Provides the specified service</summary>
    /// <typeparam name="TService">Type of service that will be provided</typeparam>
    /// <returns>An <code>std::shared_ptr</code> containing the service</returns>
    /// <remarks>
    ///   This variant will throw an exception if the service has not been registered
    ///   to the service provider and thus, it can neither provide an existing instance
    ///   nor construct a new one.
    /// </remarks>
    public: template<typename TService>
    inline std::shared_ptr<TService> GetService();

    /// <summary>Provides a factory method that creates the specified service</summary>
    /// <typeparam name="TService">Type of service that will be provided</typeparam>
    /// <returns>
    ///   A factory method that will provide an instance of the specified service
    /// </returns>
    /// <remarks>
    ///   While it is called a factory method, it still matches the behavior of
    ///   the service provider - singleton and scoped services will result in the same
    ///   instance being provided for each call. For transient services, the returned
    ///   factory method will act as a true factory and create a new instance every time.
    /// </remarks>
    public: template<typename TService>
    inline std::function<std::shared_ptr<TService>()> GetServiceFactory();

    /// <summary>Provides all instances registered for the specified service</summary>
    /// <typeparam name="TService">Type of services that will be provided</typeparam>
    /// <returns>A list of <code>std::shared_ptr</code>s containing each service</returns>
    /// <remarks>
    ///   This variant will create all services that implement the requested service
    ///   type. It is generally used when implementing plug-in like systems, i.e. to obtain
    ///   all <code>ImageLoader</code> instances or all <code>AudioFilter</code> instances
    ///   that have been registered at program launch, avoiding the necessity to write
    ///   custom plug-in hubs when all that's needed is a set of all implementations.
    /// </remarks>
    public: template<typename TService>
    inline std::vector<std::shared_ptr<TService>> GetServices();

    /// <summary>Creates a new service scope</summary>
    /// <returns>The new service scope</returns>
    /// <remarks>
    ///   <para>
    ///     This dependency injector distinguishes between global services and
    ///     &quot;scoped&quot; services. Global services share the lifetime of the service
    ///     container while scoped services exist only for as long as the scope exists,
    ///     whilst still being able to depend on global services.
    ///   </para>
    ///   <para>
    ///     Scopes are typically used to ensure that separate, independent database
    ///     connections exist per web request or open window/dialog (assuming a database
    ///     connection is provided through a &quot;scpped&quot; service). You can adapt
    ///     scopes to represent a game session or level, a script being executed and other
    ///     things depending on the kind of application you develop.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual std::shared_ptr<ServiceScope> CreateScope() = 0;

    /// <summary>Tries to provide the specified service</summary>
    /// <param name="serviceType">Type of service that will be provided</param>
    /// <returns>An <code>std::any</code> containing the service if it can be provided</returns>
    /// <remarks>
    ///   <para>
    ///     An empty <code>std::any</code> will be returned if the specified service has
    ///     not been registered to the service provider and thus, it can neither provide
    ///     an existing instance nor construct a new one.
    ///   </para>
    ///   <para>
    ///     If there is another problem, this method will still throw an exception.
    ///   </para>
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual std::any TryGetService(
      const std::type_info &typeInfo
    ) = 0;

    /// <summary>Provides the specified service</summary>
    /// <param name="serviceType">Type of service that will be provided</param>
    /// <returns>An <code>std::any</code> containing the service</returns>
    /// <remarks>
    ///   This variant will throw an exception if the service has not been registered
    ///   to the service provider and thus, it can neither provide an existing instance
    ///   nor construct a new one.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual std::any GetService(
      const std::type_info &typeInfo
    ) = 0;

    /// <summary>Provides a factory method that creates the specified service</summary>
    /// <param name="serviceType">Type of service that will be provided</param>
    /// <returns>
    ///   A factory method that will provide an instance of the specified service
    ///   as a <code>std::shared_ptr</code> wrapped in an <code>std::any</code>.
    /// </returns>
    /// <remarks>
    ///   While it is called a factory method, it still matches the behavior of
    ///   the service provider - singleton and scoped services will result in the same
    ///   instance being provided for each call. For transient services, the returned
    ///   factory method will act as a true factory and create a new instance every time.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual std::function<std::any()> GetServiceFactory(
      const std::type_info &typeInfo
    ) const = 0;

    /// <summary>Provides all instances registered for the specified service</summary>
    /// <param name="serviceType">Type of service that will be provided</param>
    /// <returns>A list of <code>std::any</code>s containing each service</returns>
    /// <remarks>
    ///   This variant will create all services that implement the requested service
    ///   type. It is generally used when implementing plug-in like systems, i.e. to obtain
    ///   all <code>ImageLoader</code> instances or all <code>AudioFilter</code> instances
    ///   that have been registered at program launch, avoiding the necessity to write
    ///   custom plug-in hubs when all that's needed is a set of all implementations.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual std::vector<std::any> GetServices(
      const std::type_info &typeInfo
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline std::shared_ptr<TService> ServiceProvider::TryGetService() {
    //typedef typename std::decay<TService>::type VanillaServiceType;
    typedef std::shared_ptr<TService> SharedServicePointer;

    std::any serviceAsAny = TryGetService(typeid(TService));
    if(serviceAsAny.has_value()) {
      return std::any_cast<SharedServicePointer>(serviceAsAny);
    } else {
      return SharedServicePointer();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline std::shared_ptr<TService> ServiceProvider::GetService() {
    typedef std::shared_ptr<TService> SharedServicePointer;
    return std::any_cast<SharedServicePointer>(GetService(typeid(TService)));
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline std::function<std::shared_ptr<TService>()> ServiceProvider::GetServiceFactory() {
    typedef std::shared_ptr<TService> SharedServicePointer;

    std::function<std::any()> anyFactory = GetServiceFactory(typeid(TService));
    return [anyFactory]() mutable -> SharedServicePointer {
      return std::any_cast<SharedServicePointer>(anyFactory());
    };
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline std::vector<std::shared_ptr<TService>> ServiceProvider::GetServices() {
    typedef std::vector<std::any> AnyVector;
    typedef std::shared_ptr<TService> SharedServicePointer;
    typedef std::vector<SharedServicePointer> SharedServicePointerVector;

    AnyVector servicesAsAny = GetServices(typeid(TService));
    std::size_t serviceCount = servicesAsAny.size();

    SharedServicePointerVector services(serviceCount);
    for(std::size_t index = 0; index < serviceCount; ++index) {
      services[index] = std::any_cast<SharedServicePointer>(servicesAsAny[index]);
    }

    return services;
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H
