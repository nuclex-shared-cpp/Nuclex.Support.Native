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

#ifndef NUCLEX_SUPPORT_SERVICES2_STANDARDINSTANCESET_H
#define NUCLEX_SUPPORT_SERVICES2_STANDARDINSTANCESET_H

#include "Nuclex/Support/Config.h"
#include "./StandardBindingSet.h"

#include <cstddef> // for std::size_t
#include <atomic> // for std::atomic<>

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores instances of created services for a service provider</summary>
  class StandardInstanceSet {

    #pragma region struct Entry

    /// <summary>Specifies the amount of a resource that a task needs to execute</summary>
    public: struct Entry {

      /// <summary>Amount of the resource (core count, bytes memory) the task needs</summary>
      public: std::size_t Amount;

      /// <summary>Kind of resource the task will occupy to do its work</summary>
      public: int Type;

    };

    #pragma endregion // struct Entry

    /// <summary>Creates a service instance set for the specified binding subset</summary>
    /// <param name="bindings">Service bindings for which instances will be stored</param>
    /// <param name="ownBindings">Binding subset to allocate instances for</param>
    /// <returns>A new service instance set</returns>
    public: static std::shared_ptr<StandardInstanceSet> Create(
      const std::shared_ptr<const StandardBindingSet> &bindings,
      const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
    );

    /// <summary>Initializes a new standard instance set</summary>
    /// <param name="bindings">Service bindings for which instances will be stored</param>
    /// <param name="ownBindings">Binding subset to allocate instances for</param>
    /// <remarks>
    ///   This constructor only works when called through Create() because it assumes that
    ///   all memory is allocated as one large block. Using this constructor normally will
    ///   cause UB, most likely a segmentation fault.
    /// </remarks>
    public: StandardInstanceSet(
      const std::shared_ptr<const StandardBindingSet> &bindings,
      const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
    );

    /// <summary>Frees all memory owned by the instance</summary>
    public: ~StandardInstanceSet();

    /// <summary>Service bindings for which instances are being stored</summary>
    public: std::shared_ptr<const StandardBindingSet> Bindings;
    /// <summary>
    ///   Bindings for which instances are managed (references either singleton bindings
    ///   or scoped bindings inside the referenced standard binding set)
    /// </summary>
    public: const StandardBindingSet::TypeIndexBindingMultiMap &OwnBindings;
    /// <summary>Mutex that must be held when updating an instance</summary>
    public: std::mutex ChangeMutex;
    /// <summary>Flag for each service that indicates whether it is present</summary>
    public: std::atomic<bool> *PresenceFlags;
    /// <summary>Instances of all services</summary>
    public: std::any *Instances;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_STANDARDINSTANCESET_H
