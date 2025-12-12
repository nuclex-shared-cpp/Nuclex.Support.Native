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

#ifndef NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H
#define NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services2/StandardServiceCollection.h"

#include <vector> // for std::vector<>
#include <functional> // for std::function<>
#include <typeinfo> // for std::type_info
#include <memory> // for std::shared_ptr<>

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Private implementation of the standard service collection</summary>
  class StandardServiceCollection::PrivateImplementation {

    /// <summary>Stores the details of a registered service binding</summary>
    public: class ServiceBinding {

      /// <summary>Initializes a new service binding for a factory-constructed service</summary>
      /// <param name="serviceType">Type of service this binding provides</param>
      /// <param name="factory">Factory function that creates an instance of the service</param>
      /// <param name="lifetime">Lifetype policy the service will follow</param>
      public: ServiceBinding(
        const std::type_info &serviceType,
        const std::function<std::any(const std::shared_ptr<ServiceProvider> &)> &factory,
        ServiceLifetime lifetime
      );

      /// <summary>Initializes a new service binding for a protoype-cloned service</summary>
      /// <param name="serviceType">Type of service this binding provides</param>
      /// <param name="prototype">Prototype that will be cloned</param>
      /// <param name="cloneFactory">Factory that will clone the protoype instance</param>
      /// <param name="lifetime">Lifetype policy the service will follow</param>
      public: ServiceBinding(
        const std::type_info &serviceType,
        const std::any &prototype,
        const std::function<std::any(const std::any &)> &cloneFactory,
        ServiceLifetime lifetime
      );

      /// <summary>Initializes a new service binding as copy of another</summary>
      /// <param name="other">Other service binding that will be copied</param>
      public: ServiceBinding(const ServiceBinding &other);
      /// <summary>Initializes a new service binding taking over another</summary>
      /// <param name="other">Other service binding that will be taken over</param>
      public: ServiceBinding(ServiceBinding &&other);

      /// <summary>Frees all resources owned by the instance</summary>
      public: ~ServiceBinding();

      /// <summary>Overwrites this service binding with a copy of another</summary>
      /// <param name="other">Other service binding that will be copied</param>
      /// <returns>A reference to the target of the assignment</returns>
      public: ServiceBinding &operator =(const ServiceBinding &other);
      /// <summary>Overwrites this service binding by taking over another</summary>
      /// <param name="other">Other service binding that will be taken over</param>
      /// <returns>A reference to the target of the assignment</returns>
      public: ServiceBinding &operator =(ServiceBinding &&other);

      /// <summary>Type of the service this binding is providing</summary>
      public: const std::type_info *ServiceType;

      /// <summary>
      ///   Existing instance (a wrapped <code>std::shared_ptr</code> of the service
      ///   type (or a class derived there in the special case of a transient binding)
      /// </summary>
      /// <remarks>
      ///   When the service binding is a transient one with an instance, this dependency
      ///   injector understand that as the protoype pattern and will require a copy
      ///   constructor that it will use to produce new instances. For any other binding,
      ///   this member is empty in the <see cref="StandardServiceCollection" /> class but
      ///   filled once an instance has been requested.
      /// </remarks>
      public: std::any ExistingInstance;

      /// <summary>
      ///   Factory method that will produce an instance of the service
      /// </summary>
      /// <remarks>
      ///   This is usually a generated factory method using template magic, but thee
      ///   are overloads that allow the user to register their own factory method,
      ///   which could effectively do anything, such as even looking up another service.
      /// </remarks>
      public: union {
        std::function<std::any(const std::shared_ptr<ServiceProvider> &)> Factory;
        std::function<std::any(const std::any &)> CloneFactory;
      };

      /// <summary>Lifetime scope for which this binding has been registered</summary>
      public: ServiceLifetime Lifetime;

    };

    /// <summary>Vector of service bindings</summary>
    public: typedef std::vector<ServiceBinding> ServiceBindingVector;

    /// <summary>Services that have been added to the service collection</summary>
    public: ServiceBindingVector Services;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H
