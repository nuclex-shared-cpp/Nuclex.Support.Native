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

#ifndef NUCLEX_SUPPORT_SERVICES_SERVICECONTAINER_H
#define NUCLEX_SUPPORT_SERVICES_SERVICECONTAINER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/ServiceProvider.h"

#include <cstddef> // for std::size_t
#include <map> // for std::map (storing services by std::type_info)

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores services provided by the application environment</summary>
  /// <remarks>
  ///   This class can be used by inversion of control containers to store and retrieve
  ///   their services. It is also possible to use it directly and store and retrieve
  ///   services manually if the application has special requirements.
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ServiceContainer : public ServiceProvider {

    #pragma region class TypeInfoComparer

    /// <summary>Compares instances of std::type_info</summary>
    private: class TypeInfoComparer {

      /// <summary>Determines the relationship of two std::type_info instances</summary>
      /// <param name="left">Type info to compare on the left side</param>
      /// <param name="right">Type info to compare on the right side</param>
      /// <returns>True if the left side comes before the right side</returns>
      public: bool operator()(const std::type_info *left, const std::type_info *right) const {
        return left->before(*right) != 0;
      }

    };

    #pragma endregion // class TypeInfoComparer

    /// <summary>Initializes a new service container</summary>
    public: NUCLEX_SUPPORT_API ServiceContainer() = default;

    /// <summary>Destroys the service container and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~ServiceContainer() = default;

    /// <summary>Counts the number of services registered in the container</summary>
    /// <returns>The number of services the container is currently holding</returns>
    public: NUCLEX_SUPPORT_API std::size_t CountServices() const {
      return this->services.size();
    }

    // Unhide the templated Get method from the service provider
    using ServiceProvider::Get;

    // Unhide the templated TryGet method fro mthe service provider
    using ServiceProvider::TryGet;

    /// <summary>Adds a service to the container</summary>
    /// <typeparam name="TService">Interface under which the service will be added</typeparam>
    /// <param name="service">Service that will be responsible for the interface</param>
    public: template<typename TService> void Add(const std::shared_ptr<TService> &service) {
      Add(typeid(TService), std::any(service));
    }

    /// <summary>Removes a service from the container</summary>
    /// <typeparam name="TService">Interface of the service that will be removed</typeparam>
    /// <returns>True if the service existed and was removed</returns>
    public: template<typename TService> bool Remove() {
      return Remove(typeid(TService));
    }

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="std::any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API const std::any &Get(
      const std::type_info &serviceType
    ) const;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>An std::any containing the service, if found, or an empty std::any</returns>
    /// <remarks>
    ///   <para>
    ///     An empty <see cref="std::any" /> will be returned if the specified service has not
    ///     been activated yet (for a mere container, that means it's not in the container,
    ///     for a factory, it means it has not been constructed yet or its lifetime requires
    ///     that the service instance is not stored).
    ///   </para>
    ///   <para>
    ///     If there is another problem, this method will still throw an exception.
    ///   </para>
    /// </remarks>
    protected: NUCLEX_SUPPORT_API const std::any &TryGet(const std::type_info &serviceType) const;

    /// <summary>Adds a service to the container</summary>
    /// <param name="serviceType">
    ///   Type of the service that will be added to the container
    /// </param>
    /// <param name="service">Object that provides the service</param>
    protected: NUCLEX_SUPPORT_API void Add(
      const std::type_info &serviceType, const std::any &service
    );

    /// <summary>Removes a service from the container</summary>
    /// <param name="serviceType">
    ///   Type of the service that will be removed from the container
    /// </param>
    /// <returns>True if the service was found and removed</returns>
    protected: NUCLEX_SUPPORT_API bool Remove(const std::type_info &serviceType);

    /// <summary>Map through which services can be looked up</summary>
    private: typedef std::map<const std::type_info *, std::any, TypeInfoComparer> ServiceMap;

    /// <summary>Services the container is currently holding</summary>
    private: ServiceMap services;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICECONTAINER_H
