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

#ifndef NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H
#define NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Services2/ServiceLifetime.h" // for ServiceLifetime

#include <cstddef> // for std::size_t
#include <memory> // for std::shared_ptr
#include <type_traits> // for std::decay, std::type_info
#include <any> // for std::any
#include <functional> // for std::function<>
#include <utility> // for std::index_sequence<>, std::make_index_sequence<>

namespace Nuclex::Support::Services2 {
  class ServiceProvider;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores configured services and can build service providers</summary>
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
  ///   <para>
  ///     Note that this is just the interface for the service collection. Typically, you'd
  ///     work with the <see cref="StandardServiceCollection" /> and pass it by this
  ///     interface the service binding setup code.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ServiceCollection {

    /// <summary>Frees all resources owned by the service collection</summary>
    public: NUCLEX_SUPPORT_API virtual ~ServiceCollection();

    /// <summary>
    ///   Adds a singleton service where the service interface and implementation class are
    ///   one and the same.
    /// </summary>
    /// <typeparam name="TServiceAndImplementation">
    ///   Type that represents both the service interface and its implementation
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TServiceAndImplementation>
    inline ServiceCollection &AddSingleton();

    /// <summary>
    ///   Adds a singleton service with a service interface and a separate implementation
    ///   that will provide the service when it is requested by its interface.
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface, typically containing only pure virtuals.
    /// </typeparam>
    /// <typeparam name="TImplementation">
    ///   Type that implements the service, must inherit <code>TService</code>.
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService, typename TImplementation>
    inline ServiceCollection &AddSingleton();

    /// <summary>
    ///   Adds a singleton service that uses a custom factory method to construct it
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since construction happens via
    ///   a factory method, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="factory">
    ///   Factory method that will be called to construct an instance of the service
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddSingleton(
      const std::function<
        std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
      > &factory
    );

    /// <summary>Adds a singleton service that exposes an already existing instance</summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since an existing instance is being
    ///   provided, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="instance">
    ///   Already existing service instance that has been externally constructed
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddSingleton(const std::shared_ptr<TService> &instance);

    /// <summary>
    ///   Adds a scoped service where the service interface and implementation class are
    ///   one and the same.
    /// </summary>
    /// <typeparam name="TServiceAndImplementation">
    ///   Type that represents both the service interface and its implementation
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TServiceAndImplementation>
    inline ServiceCollection &AddScoped();

    /// <summary>
    ///   Adds a scoped service with a service interface and a separate implementation
    ///   that will provide the service when it is requested by its interface.
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface, typically containing only pure virtuals.
    /// </typeparam>
    /// <typeparam name="TImplementation">
    ///   Type that implements the service, must inherit <code>TService</code>.
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService, typename TImplementation>
    inline ServiceCollection &AddScoped();

    /// <summary>
    ///   Adds a scoped service that uses a custom factory method to construct it
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since construction happens via
    ///   a factory method, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="factory">
    ///   Factory method that will be called to construct an instance of the service
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddScoped(
      const std::function<
        std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
      > &factory
    );

    /// <summary>Adds a scoped service that exposes an already existing instance</summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since an existing instance is being
    ///   provided, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="instance">
    ///   Already existing service instance that has been externally constructed
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddScoped(const std::shared_ptr<TService> &instance);

    /// <summary>
    ///   Adds a transient service where the service interface and implementation class are
    ///   one and the same.
    /// </summary>
    /// <typeparam name="TServiceAndImplementation">
    ///   Type that represents both the service interface and its implementation
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TServiceAndImplementation>
    inline ServiceCollection &AddTransient();

    /// <summary>
    ///   Adds a transient service with a service interface and a separate implementation
    ///   that will provide the service when it is requested by its interface.
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface, typically containing only pure virtuals.
    /// </typeparam>
    /// <typeparam name="TImplementation">
    ///   Type that implements the service, must inherit <code>TService</code>.
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService, typename TImplementation>
    inline ServiceCollection &AddTransient();

    /// <summary>
    ///   Adds a transient service that uses a custom factory method to construct it
    /// </summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since construction happens via
    ///   a factory method, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="factory">
    ///   Factory method that will be called to construct an instance of the service
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddTransient(
      const std::function<
        std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
      > &factory
    );

    /// <summary>Adds a transient service that exposes an already existing instance</summary>
    /// <typeparam name="TService">
    ///   Type that acts as the service interface. Since an existing instance is being
    ///   provided, it can be an interface or the actual implementation class.
    /// </typeparam>
    /// <param name="instance">
    ///   Already existing service instance that has been externally constructed
    /// </typeparam>
    /// <returns>A self-reference to allow calls to be chained if desired</returns>
    public: template<typename TService>
    inline ServiceCollection &AddTransient(const std::shared_ptr<TService> &instance);

    /// <summary>Removes all service bindings for the specified type</summary>
    /// <typeparam name="TService">Type of service whose bindings will be removed</typeparam>
    /// <returns>The number of removed service bindings for the service type</returns>
    public: template<typename TService> inline std::size_t RemoveAll();

    /// <summary>Uses the services registered so far to build a service provider</summary>
    /// <returns>The new service provider</returns>
    public: NUCLEX_SUPPORT_API virtual std::shared_ptr<
      ServiceProvider
    > BuildServiceProvider() const = 0;

    /// <summary>Removes all service bindings for the specified type</summary>
    /// <param name="serviceType">Type of service whose bindings will be removed</param>
    /// <returns>The number of removed service bindings for the service type</returns>
    protected: NUCLEX_SUPPORT_API virtual std::size_t RemoveAll(
      const std::type_info &serviceType
    ) = 0;

    /// <summary>Adds the specified service binding to the collection</summary>
    /// <param name="serviceType">Type of service that will be bound</param>
    /// <param name="factoryMethod">
    ///   Method through which instances of the service will be created
    /// </param>
    /// <param name="lifetime">
    ///   Which lifetime category the service will use: singleton, scoped or transient
    /// </param>
    protected: NUCLEX_SUPPORT_API virtual void AddServiceBinding(
      const std::type_info &serviceType,
      std::function<std::any(const std::shared_ptr<ServiceProvider> &)> factoryMethod,
      ServiceLifetime lifetime
    ) = 0;

    // TODO: Investigate supporting transient instances via protoype pattern
    //   If, at registration time, it could be verified that the existing instance
    //   implements a copy constructor, a factory method could be generated that
    //   clones the existing instance, thus allowing transient instances again.

    /// <summary>Adds the specified service binding to the collection</summary>
    /// <param name="serviceType">Type of service that will be bound</param>
    /// <param name="existingInstance">
    ///   Existing instance that will be provided when the service is requested
    /// </param>
    /// <param name="lifetime">
    ///   Which lifetime category the service will use: singleton or scoped.
    /// </param>
    /// <remarks>
    ///   This method rejects <see cref="ServiceLifetime.Transient" /> because it
    ///   has no way to create new instances from the existing instance.
    /// </remarks>
    protected: NUCLEX_SUPPORT_API virtual void AddServiceBinding(
      const std::type_info &serviceType,
      const std::any &existingInstance,
      ServiceLifetime lifetime
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}

#include "Private/IsSharedPtr.inl"
#include "Private/IsInjectableType.inl"
#include "Private/IsServiceInstanceType.inl"

#include "Private/ConstructorSignature.inl"
#include "Private/ConstructorSignatureDetector.inl"

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  template<typename TServiceAndImplementation>
  inline ServiceCollection &ServiceCollection::AddSingleton() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService, typename TImplementation>
  inline ServiceCollection &ServiceCollection::AddSingleton() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddSingleton(
    const std::function<
      std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
    > &factory
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddSingleton(
    const std::shared_ptr<TService> &instance
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TServiceAndImplementation>
  inline ServiceCollection &ServiceCollection::AddScoped() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService, typename TImplementation>
  inline ServiceCollection &ServiceCollection::AddScoped() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddScoped(
    const std::function<
      std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
    > &factory
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddScoped(
    const std::shared_ptr<TService> &instance
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TServiceAndImplementation>
  inline ServiceCollection &ServiceCollection::AddTransient() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService, typename TImplementation>
  inline ServiceCollection &ServiceCollection::AddTransient() {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddTransient(
    const std::function<
      std::shared_ptr<TService>(const std::shared_ptr<ServiceProvider> &)
    > &factory
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline ServiceCollection &ServiceCollection::AddTransient(
    const std::shared_ptr<TService> &instance
  ) {
    // TODO: Here comes the complex part, constructor signature detection and factory creation
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TService>
  inline std::size_t ServiceCollection::RemoveAll() {
    return RemoveAll(typeid(TService));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H
