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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Event.h"

#include <cstddef> // for std::size_t

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides notifications for when an indexed collection changes</summary>
  /// <remarks>
  ///   <para>
  ///     This does not monitor the state of items stored in the collection, it just reports
  ///     when the items are added, removed or replaced in the collection.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class ObservableIndexedCollection {

    /// <summary>Fired when an item has been added to the collection</summary>
    /// <param name="index">Index at which the item has been added</param>
    /// <param name="value">Item that has been added to the collection</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &value)
    > ItemAdded;

    /// <summary>Fired when an item has been removed from the collection</summary>
    /// <param name="index">Index at which the item has been removed</param>
    /// <param name="value">Item that has been removed from the collection</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &value)
    > ItemRemoved;

    /// <summary>Fired when an item in the collection has been replaced</summary>
    /// <param name="index">Index at which the item has been replaced</param>
    /// <param name="oldValue">Item that is no longer part of the collection</param>
    /// <param name="newValue">Item that has taken the place of the old item</param>
    public: mutable Events::Event<
      void(std::size_t index, const TValue &oldValue, const TValue &newValue)
    > ItemReplaced;

    // public: mutable Event Clearing();
    // public: mutable Event Cleared();

    /// <summary>Initializes a new observable indexed collection</summary>
    protected: ObservableIndexedCollection() = default;
    /// <summary>Frees all memory used by the observable collection</summary>
    public: virtual ~ObservableIndexedCollection() = default;

    //private: ObservableIndexedCollection(const ObservableIndexedCollection &) = delete;
    //private: ObservableIndexedCollection &operator =(
    //  const ObservableIndexedCollection &
    //) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLEINDEXEDCOLLECTION_H
