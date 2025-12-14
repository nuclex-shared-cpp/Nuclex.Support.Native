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

#ifndef NUCLEX_SUPPORT_SERVICES2_STANDARDBINDINGSET_H
#define NUCLEX_SUPPORT_SERVICES2_STANDARDBINDINGSET_H

#include "Nuclex/Support/Config.h"

#include <typeindex> // for std::type_index
#include <functional> // for std::function<>
#include <any> // for std::any
#include <memory> // for std::shared_ptr<>
#include <map> // for std::multimap<>

namespace Nuclex::Support::Services2 {
  class ServiceProvider;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores the service bindings set up for a service provider</summary>
  class StandardBindingSet {

    #pragma region class Binding

    /// <summary>Stores the details of a registered service binding</summary>
    public: class Binding {

      /// <summary>Initializes a new service binding for a factory-constructed service</summary>
      /// <param name="factory">Factory function that creates an instance of the service</param>
      public: explicit Binding(
        const std::function<std::any(ServiceProvider &)> &factory
      );

      /// <summary>Initializes a new service binding for a protoype-cloned service</summary>
      /// <param name="prototype">Prototype that will be cloned</param>
      /// <param name="cloneFactory">Factory that will clone the protoype instance</param>
      public: explicit Binding(
        const std::any &prototype,
        const std::function<std::any(const std::any &)> &cloneFactory
      );

      /// <summary>Initializes a new service binding as copy of another</summary>
      /// <param name="other">Other service binding that will be copied</param>
      public: Binding(const Binding &other);

      /// <summary>Initializes a new service binding taking over another</summary>
      /// <param name="other">Other service binding that will be taken over</param>
      public: Binding(Binding &&other);

      /// <summary>Frees all resources owned by the instance</summary>
      public: ~Binding();

      /// <summary>Overwrites this service binding with a copy of another</summary>
      /// <param name="other">Other service binding that will be copied</param>
      /// <returns>A reference to the target of the assignment</returns>
      public: Binding &operator =(const Binding &other);

      /// <summary>Overwrites this service binding by taking over another</summary>
      /// <param name="other">Other service binding that will be taken over</param>
      /// <returns>A reference to the target of the assignment</returns>
      public: Binding &operator =(Binding &&other);

      /// <summary>Unique index of the service in a service set</summary>
      /// <remarks>
      ///   When used with the service collection
      /// </remarks>
      public: std::size_t UniqueServiceIndex;

      /// <summary>
      ///   Existing instance (a wrapped <code>std::shared_ptr</code> of the service type
      /// </summary>
      /// <remarks>
      ///   For service bindings where the user has provided their own instance of a service,
      ///   this will store said instance. For singleton and scoped services, it will also
      ///   be the instance that gets handed out via the service provider. For transient
      ///   services that return a new instance on each request, this acts as a prototype
      ///   instance which is cloned (via its copy constructor) to generate new instances.
      /// </remarks>
      public: std::any ProvidedInstance;

      /// <summary>
      ///   Factory method that will produce an instance of the service
      /// </summary>
      /// <remarks>
      ///   <para>
      ///     This is usually a generated factory method using template magic, but there
      ///     are overloads that allow the user to register their own factory method,
      ///     which could effectively do anything, such as even looking up another service.
      ///   </para>
      ///   <para>
      ///     The clone factory works is filled for all services where the prototype or existing
      ///     instance is non-zero. Instead of a service provider, it expects the prototype
      ///     so it can create a new service instance by cloning it.
      ///   </para>
      /// </remarks>
      public: union {
        std::function<std::any(ServiceProvider &)> Factory;
        std::function<std::any(const std::any &)> CloneFactory;
      };

    };
    
    #pragma endregion // class Binding

    /// <summary>Generates unique indices for all singleton and scoped services</summary>
    /// <remarks>
    ///   The unique indices allow the service providers to store the actual service
    ///   instances inside of plain arrays.
    /// </remarks>
    public: void GenerateUniqueIndexes();

    /// <summary>A map from types to their service bindings</summary>
    public: typedef std::multimap<std::type_index, Binding> TypeIndexBindingMultiMap;

    /// <summary>Singleton services that have been added to the service collection</summary>
    public: TypeIndexBindingMultiMap SingletonServices;
    /// <summary>Scoped services that have been added to the service collection</summary>
    public: TypeIndexBindingMultiMap ScopedServices;
    /// <summary>Transient services that have been added to the service collection</summary>
    public: TypeIndexBindingMultiMap TransientServices;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_STANDARDBINDINGSET_H
