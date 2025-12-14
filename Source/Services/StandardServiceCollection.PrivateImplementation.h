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

#ifndef NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H
#define NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services/StandardServiceCollection.h"

#include "./StandardBindingSet.h"

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Private implementation of the standard service collection</summary>
  class StandardServiceCollection::PrivateImplementation {

    /// <summary>Service bindings that have been added to the service collection</summary>
    public: StandardBindingSet Bindings;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services

#endif // NUCLEX_SUPPORT_SERVICES_STANDARDSERVICECOLLECTION_PRIVATEIMPLEMENTATION_H
