#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceContainer.h"
#include "Nuclex/Support/Services/ServiceLifetime.h"
#include "Nuclex/Support/Events/Delegate.h"

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The maximum number of constructor arguments that can be injected</summary>
  /// <remarks>
  ///   Increasing this value will result in (slightly) slower compiles. Though you might
  ///   want to reconsider your design if a single type consumes more than 8 services ;)
  /// </remarks>
  static constexpr std::size_t MaximumConstructorArgumentCount = 8;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#include "Nuclex/Support/Services/IntegerSequence.inl"
#include "Nuclex/Support/Services/Checks.inl"
#include "Nuclex/Support/Services/ConstructorSignature.inl"
#include "Nuclex/Support/Services/ConstructorSignatureDetector.inl"
#include "Nuclex/Support/Services/ServiceFactory.inl"

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

      /// <summary>Type of a factory method for this service</summary>
      public: typedef Events::Delegate<std::shared_ptr<TService>(void)> FactoryMethodType;

      /// <summary>Binds the service to a constructor-injected provider</summary>
      /// <typeparam name="TImplementation">Implementation of the service to use</typeparam>
      /// <remarks>
      ///   This binds the service to the specified service implementation
      /// </remarks>
      public: template<typename TImplementation> void To() {
        typedef Private::DetectConstructorSignature<TImplementation> ConstructorSignature;

        static_assert(
          std::is_base_of<TService, TImplementation>::value,
          "Implementation must inherit from the service interface"
        );

        constexpr bool implementationHasInjectableConstructor = !std::is_base_of<
          Private::InvalidConstructorSignature, ConstructorSignature
        >::value;
        static_assert(
          implementationHasInjectableConstructor,
          "Implementation must have a constructor that can be dependency-injected "
          "(either providing a default constructor or using only std::shared_ptr arguments)"
        );

        throw std::logic_error("Not implemented yet");
      }

      /// <summary>Binds the service to a factory method or functor used to create it</summary>
      /// <param name="factoryMethod">
      ///   Factory method that will be called to create the service
      /// </param>
      public: void ToFactoryMethod(const FactoryMethodType &factoryMethod) {
        throw std::logic_error("Not implemented yet");
      }

      /// <summary>Binds the service to an already constructed service instance</summary>
      /// <param name="instance">Instance that will be returned for the service</param>
      public: void ToInstance(const std::shared_ptr<TService> &instance) {
        throw std::logic_error("Not implemented yet");
      }

      /// <summary>Assumes that the service and its implementation are the same type</summary>
      /// <remarks>
      ///   For trivial services that don't have an interface separate from their implementation
      ///   class (or when you just have to provide some implementation everywhere),
      ///   use this method to say that the service type is a non-abstract class and
      ///   should be created directly.
      /// </remarks>
      public: void ToSelf() {
        typedef Private::DetectConstructorSignature<TService> ConstructorSignature;

        constexpr bool serviceHasInjectableConstructor = !std::is_base_of<
          Private::InvalidConstructorSignature, ConstructorSignature
        >::value;
        static_assert(
          serviceHasInjectableConstructor,
          "Self-bound service must not be abstract and requires a constructor "
          "that can be dependency-injected (either providing a default constructor or "
          "using only std::shared_ptr arguments)"
        );

        ServiceProvider *needThis = nullptr;
        Private::ServiceFactory<TService, ConstructorSignature>::CreateInstance(*needThis);

        throw std::logic_error("Not implemented yet");
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
      /// <returns>An Any containing the service, if found, or an empty Any</returns>
      public: const Any &TryGet(const std::type_info &serviceType) const {
        return ServiceContainer::TryGet(serviceType);
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
        return ServiceContainer::Remove(serviceType);
      }

    };

    #pragma endregion // class ServiceStore

    /// <summary>Initializes a new service injector</summary>
    public: NUCLEX_SUPPORT_API LazyServiceInjector() = default;

    /// <summary>Destroys the service injector and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~LazyServiceInjector() = default;

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
    ) const override;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>An Any containing the service, if found, or an empty Any</returns>
    protected: NUCLEX_SUPPORT_API const Any &TryGet(
      const std::type_info &serviceType
    ) const override;

    //private: void 

    /// <summary>Stores services that have already been initialized</summary>
    private: ServiceStore services;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H
