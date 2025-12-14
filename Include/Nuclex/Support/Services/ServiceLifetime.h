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

#ifndef NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H
#define NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H

#include "Nuclex/Support/Config.h"

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Different lifetime types for which services can be registered</summary>
  enum class NUCLEX_SUPPORT_TYPE ServiceLifetime {

    /// <summary>
    ///   Only one instance of the service will be created per service provider and
    ///   it will continue to exist until the service provider is destroyed
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     Use this lifetime for application-global services such as as an application
    ///     settings service that exposes the contents of a settings file, or a directory
    ///     lookup service etc.
    ///   </para>
    ///   <para>
    ///     Depending on the application, application-level workers and managers can also
    ///     take a role in creating service scopes. An HTTP server, for example, might
    ///     set up a scope for each request it handles so that scoped services use a unique
    ///     database connection (assuming the database service is a scoped service).
    ///   </para>
    ///   <para>
    ///     Singleton services can depend on other singleton services, but not on scoped
    ///     services because those have a shorter lifetime.
    ///   </para>
    /// </remarks>
    Singleton,

    /// <summary>
    ///   Instances can only be requested from inside a service and one instance will be
    ///   created per service scope that exists until the service scope is destroyed
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     It is up to you to which concept you map scopes in your application. A typical
    ///     web application would have one scope per handled request (so that scoped services
    ///     for database access or user identify / session cookie access are separate for
    ///     each service scope). A desktop application might want to use a window manager
    ///     that sets up a service scope per window or dialog for the same reasons.
    ///   </para>
    ///   <para>
    ///     usually, having the concept of scopes in general is enough, even if you have
    ///     different concepts of scope users. A background worker in a web application would
    ///     use a scope but request the only database access service, not the cookie service.
    ///   </para>
    ///   <para>
    ///     Scoped services can depend on singleton services (that exist on the level of
    ///     the global service provider) and on other scoped services (that will be created
    ///     within the same scope).
    ///   </para>
    /// </remarks>
    Scoped,

    /// <summary>
    ///   Transient services are fire-and-forget services of which a new instance is created
    ///   whenever the service is requested. Their lifetime is under your control.
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     Transient services are useful if your want to use the dependency injector like
    ///     an abstract factory to create a new instance of a certain type or interface on
    ///     demand. Typical use cases are view models in an MVVM application or background
    ///     workers, both of which could depend on any of your registered services via
    ///     automatic constructor injection.
    ///   </para>
    ///   <para>
    ///     Transient services can be requested at any level. A transient instance requested
    ///     from a singleton will have access to all singleton services and a transient
    ///     instance requested from within a service scope will additionalle have access to
    ///     any of the service the scope can provide.
    ///   </para>
    /// </remarks>
    Transient

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_SERVICELIFETIME_H
