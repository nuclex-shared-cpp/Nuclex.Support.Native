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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_MAP_H
#define NUCLEX_SUPPORT_COLLECTIONS_MAP_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interface for a container that maps keys to values</summary>
  /// <typeparam name="TKey">Type of the key the map uses</typeparam>
  /// <typeparam name="TValue">Type of values that are stored in the map</typeparam>
  template<typename TKey, typename TValue>
  class Map {

    /// <summary>Destroys the map</summary>
    public: virtual ~Map() = default;

    /// <summary>Stores a value in the map</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key in the map</param>
    /// <returns>
    ///   True if the key did not exist before and was inserted,
    ///   false if the key already existed and its value was replaced.
    /// </returns>
    public: virtual bool Insert(const TKey &key, const TValue &value) = 0;

    /// <summary>Stores a value in the map if it doesn't exist yet</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key in the map</param>
    /// <returns>
    ///   True if the key did not exist before and was inserted,
    ///   false if the key already existed and left unchanged
    /// </returns>
    public: virtual bool TryInsert(const TKey &key, const TValue &value) = 0;

    /// <summary>Returns the value of the specified element in the map</summary>
    /// <param name="key">Key of the element that will be looked up</param>
    public: virtual const TValue &Get(const TKey &key) const = 0;
    
    // TODO: Code smell - TryGet() couldn't be adapted to Get()
    //   Can't pinpoint the design flaw, if the interface design is cohesive,
    //   such situations should resolve themselves, something is suboptimal here.

    /// <summary>Tries to look up an element in the map</summary>
    /// <param name="key">Key of the element that will be looked up</param>
    /// <param name="value">Will receive the value if the element was found</param>
    /// <returns>
    ///   True if an element was returned, false if the key didn't exist
    /// </returns>
    public: virtual bool TryGet(const TKey &key, TValue &value) const = 0;

    /// <summary>Tries to take an element from the map (removing it)</summary>
    /// <param name="key">Key of the element that will be taken from the map</param>
    /// <param name="value">Will receive the value taken from the map</param>
    /// <returns>
    ///   True if an element was found and removed from the map, false if the key didn't exist
    /// </returns>
    public: virtual bool TryTake(const TKey &key, TValue &value) = 0;

    // CHECK: Could provide a TryTake() with output method to allow for move semantics.
    //public: virtual bool TryTake(const TKey &key, const std::function<TValue &&> &valueCallback) = 0;

    /// <summary>Removes the specified element from the map if it exists</summary>
    /// <param name="key">Key of the element that will be removed if present</param>
    /// <returns>True if the element was found and removed, false otherwise</returns>
    public: virtual bool TryRemove(const TKey &key) = 0;

    /// <summary>Removes all items from the map</summary>
    public: virtual void Clear() = 0;

    /// <summary>Counts the numebr of elements currently in the map</summary>
    /// <returns>
    ///   The approximate number of elements that had been in the map during the call
    /// </returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the map is empty</summary>
    /// <returns>True if the map had been empty during the call</returns>
    public: virtual bool IsEmpty() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_MAP_H
