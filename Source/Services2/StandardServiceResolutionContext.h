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

#ifndef NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICERESOLUTIONCONTEXT_H
#define NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICERESOLUTIONCONTEXT_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services2/ServiceProvider.h"

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Interface for the service provider proxy used to resolve each dependency chain
  /// </summary>
  class StandardServiceResolutionContext : public ServiceProvider {

    /// <summary>Frees all resources owned by the service resolution context</summary>
    public: virtual ~StandardServiceResolutionContext() override;

    /// <summary>Reports the level of dependency recursions the resolution is at</summary>
    /// <returns>The number of recursive dependency requests</returns>
    /// <remarks>
    ///   For the outermost request, directly issues from user code, this value is zero.
    ///   This is used by the <see cref="StandardInstanceSet" /> to detect whether
    ///   service construction needs to acquire the mutex lock or not. It is provable that
    ///   this lock is either required at the root level or that the entire dependency
    ///   graph has already been constructed (otherwise the instance couldn't exist).
    /// </remarks>
    public: virtual std::size_t GetResolutionDepth() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_STANDARDSERVICERESOLUTIONCONTEXT_H
