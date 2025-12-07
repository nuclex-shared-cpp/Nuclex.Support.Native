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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Event.h"

//#include <cstdlib>

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides notifications for when a collection changes</summary>
  /// <remarks>
  ///   <para>
  ///     This does not monitor the state of items stored in the collection, it just reports
  ///     when items are added, removed or replaced in the collection.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class ObservableCollection {

    /// <summary>Fired when an item has been added to the collection</summary>
    /// <param name="value">Item that has been added to the collection</param>
    public: mutable Events::Event<void(const TValue &value)> ItemAdded;
    /// <summary>Fired when an item has beenremoved from the collection</summary>
    /// <param name="value">Item that has been removed from the collection</param>
    public: mutable Events::Event<void(const TValue &value)> ItemRemoved;

    // public: mutable Event Clearing();
    // public: mutable Event Cleared();

    /// <summary>Initializes a new observable collection</summary>
    protected: ObservableCollection() = default;
    /// <summary>Frees all memory used by the observable collection</summary>
    public: virtual ~ObservableCollection() = default;

    //private: ObservableCollection(const ObservableCollection &) = delete;
    //private: ObservableCollection &operator =(const ObservableCollection &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H
