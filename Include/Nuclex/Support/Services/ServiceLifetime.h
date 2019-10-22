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
    ///   The lifetime of the service will end after it has been requested
    /// </summary>
    /// <remarks>
    ///   This is the default for any ad-hoc services. An instance of the service
    ///   implementation will be created and handed to the user without storing
    ///   it anywhere else. Thus, each request returns a new instance.
    /// </remarks>
    Request,

    /// <summary>
    ///   The service will live in the service container as long as the injector exists
    /// </summary>
    /// <remarks>
    ///   This is the default for registered services. When an instance of the service
    ///   implementation is created, it will be stored by the injector for future
    ///   requests asking for the same service. Thus, service implementations continue
    ///   to live on until the injector is destroyed.
    /// </remarks>
    Container

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H
