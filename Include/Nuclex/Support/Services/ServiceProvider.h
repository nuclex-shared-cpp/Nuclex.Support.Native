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

#ifndef NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H

#include "../Config.h"
#include "../Any.h"

#include <memory>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides services to the application</summary>
  /// <remarks>
  ///   This is an interface through which services can be looked up. It is either used
  ///   manually (but beware of the service locator anti-pattern!) or as part of
  ///   a dependency injection framework.
  /// </remarks>
  class ServiceProvider {

    /// <summary>Destroys the service provider and frees all resources</summary>
    public: NUCLEX_SUPPORT_API virtual ~ServiceProvider() {}

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    public: template<typename TService> const std::shared_ptr<TService> &Get() {
      return Get(typeid(TService)).Get<TService>();
    }

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <param name="service">Shared pointer that will receive the service if found</param>
    /// <returns>True if the specified service was found and retrieved</returns>
    public: template<typename TService> bool TryGet(std::shared_ptr<TService> &service) {
      Any serviceAsAny;
      if(TryGet(typeid(TService), serviceAsAny)) {
        service = serviceAsAny.Get<TService>();
        return true;
      } else {
        service.reset();
        return false;
      }
    }

    /// <summary>Looks up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <returns>
    ///   The specified service as a shared_ptr wrapped in an <see cref="Any" />
    /// </returns>
    protected: NUCLEX_SUPPORT_API virtual const Any &Get(
      const std::type_info &serviceType
    ) const = 0;

    /// <summary>Tries to look up the specified service</summary>
    /// <param name="serviceType">Type of service that will be looked up</param>
    /// <param name="service">Any that will receive the shared_ptr to the service</param>
    /// <returns>True if the service was found and stored in the any</returns>
    protected: NUCLEX_SUPPORT_API virtual bool TryGet(
      const std::type_info &serviceType, Any &service
    ) const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICEPROVIDER_H
