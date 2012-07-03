#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2012 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H
#define NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H

#include "../Config.h"
#include "ServiceContainer.h"

#include <functional>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Binds services and initializes them via constructor injection</summary>
  /// <remarks>
  ///   This is a very simplified dependency injector that only supports global services
  ///   stored in shared_ptrs.
  /// </remarks>
  class LazyServiceInjector : public ServiceProvider {

    #pragma region class BindSyntax

    /// <summary>Provides the syntax for the fluent Bind() method</summary>
    public: template<typename TService> class BindSyntax {

      /// <summary>Binds the service to a constructor-injected provider</summary>
      /// <typeparam name="TProvider">Provider the service will be bound to</typeparam>
      public: template<typename TProvider> void To() {
      }

      /// <summary>Binds the service to a factory method or functor used to create it</summary>
      public: void ToMethod(const std::function<TService *()> &factory) {
        
      }

    };

    #pragma endregion // class BindSyntax

    #pragma region class ServiceStore

    /// <summary>Factory used to construct services or provide existing services</summary>
    private: class ServiceStore : public ServiceContainer {

      /// <summary>Frees all resources used by the service factory</summary>
      public: virtual ~ServiceStore() {}

      /// <summary>Looks up the specified service</summary>
      /// <param name="serviceType">Type of service that will be looked up</param>
      /// <returns>
      ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
      /// </returns>
      public: const Any &Get(const std::type_info &serviceType) const {
        return ServiceContainer::Get(serviceType);
      }

      /// <summary>Tries to look up the specified service</summary>
      /// <param name="serviceType">Type of service that will be looked up</param>
      /// <param name="service">Any that will receive the shared_ptr to the service</param>
      /// <returns>True if the service was found and stored in the any</returns>
      public: bool TryGet(const std::type_info &serviceType, Any &service) const {
        return ServiceContainer::TryGet(serviceType, service);
      }

      /// <summary>Adds a service to the container</summary>
      /// <param name="serviceType">
      ///   Type of the service that will be added to the container
      /// </param>
      /// <param name="service">Object that provides the service</param>
      public: void Add(const std::type_info &serviceType, const Any &service) {
        ServiceContainer::Add(serviceType, service);
      }

      /// <summary>Removes a service from the container</summary>
      /// <param name="serviceType">
      ///   Type of the service that will be removed from the container
      /// </param>
      /// <returns>True if the service was found and removed</returns>
      public: bool Remove(const std::type_info &serviceType) {
        ServiceContainer::Remove(serviceType);
      }

    };

    #pragma endregion // class ServiceStore

    /// <summary>Initializes a new service injector</summary>
    public: NUCLEX_SUPPORT_API LazyServiceInjector() {}

    /// <summary>Destroys the service injector and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~LazyServiceInjector() {}

    /// <summary>Binds a provider to the specified service</summary>
    /// <returns>A syntax through which the provider to be bound can be selected</returns>
    public: template<typename TService> BindSyntax<TService> Bind() {
      return BindSyntax<TService>();
    }

    // Unhide the templated Get method from the service provider
    using ServiceProvider::Get;

    // Unhide the templated TryGet method fro mthe service provider
    using ServiceProvider::TryGet;

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API const Any &Get(
      const std::type_info &serviceType
    ) const;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <param name="service">Any that will receive the shared_ptr to the service</param>
    /// <returns>True if the service was found and stored in the any</returns>
    protected: NUCLEX_SUPPORT_API bool TryGet(
      const std::type_info &serviceType, Any &service
    ) const;

    /// <summary>Stores services that have already been initialized</summary>
    private: ServiceStore services;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H
