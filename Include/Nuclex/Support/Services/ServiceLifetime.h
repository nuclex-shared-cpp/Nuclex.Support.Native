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

#ifndef NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H
#define NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H

#include "Nuclex/Support/Config.h"

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Different ways a service's lifetime can be managed by an injector</summary>
  enum class ServiceLifetime {

    /// <summary>
    ///   The service injector will use a new instance of the service for each request
    /// </summary>
    /// <remarks>
    ///   With this lifetime policy, the service injector will not keep the service around
    ///   after a request has been served.
    /// </remarks>
    Request,

    /// <summary>
    ///   The service will live in the service container as long as the injector exists
    /// </summary>
    /// <remarks>
    ///   This is the default for registered services. When an instance of the service
    ///   implementation is created, it will be stored by the injector for future
    ///   requests asking for the same service. Thus, any number of users inside your
    ///   code will be accessing the same service instance.
    /// </remarks>
    Container

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H
