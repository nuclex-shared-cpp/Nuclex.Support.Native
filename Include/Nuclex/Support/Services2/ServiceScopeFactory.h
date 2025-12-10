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

#ifndef NUCLEX_SUPPORT_SERVICES2_SERVICESCOPEFACTORY_H
#define NUCLEX_SUPPORT_SERVICES2_SERVICESCOPEFACTORY_H

#include "Nuclex/Support/Config.h"

#include <memory> // for std::shared_ptr

namespace Nuclex::Support::Services2 {
  class ServiceScope;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allows the construction of new service scopes</summary>
  /// <remarks>
  ///   Using this interface, the scope factory can be exposed on its own, without demanding
  ///   the entire service provider interface. It can be used to slim the requirements of
  ///   a background worker or other tool that needs to create service scopes.
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ServiceScopeFactory {

    /// <summary>Frees all resources owned by the service scope factory</summary>
    public: virtual ~ServiceScopeFactory();

    /// <summary>Creates a new service scope</summary>
    /// <returns>The new service scope</returns>
    /// <remarks>
    ///   <para>
    ///     This dependency injector distinguishes between global services and
    ///     &quot;scoped&quot; services. Global services share the lifetime of the service
    ///     container while scoped services exist only for as long as the scope exists,
    ///     whilst still being able to depend on global services.
    ///   </para>
    ///   <para>
    ///     Scopes are typically used to ensure that separate, independent database
    ///     connections exist per web request or open window/dialog (assuming a database
    ///     connection is provided through a &quot;scpped&quot; service). You can adapt
    ///     scopes to represent a game session or level, a script being executed and other
    ///     things depending on the kind of application you develop.
    ///   </para>
    /// </remarks>
    public: virtual std::shared_ptr<ServiceScope> CreateScope() = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_SERVICESCOPEFACTORY_H
