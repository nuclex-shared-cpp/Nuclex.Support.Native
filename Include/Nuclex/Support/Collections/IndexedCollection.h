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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_INDEXEDCOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_INDEXEDCOLLECTION_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Collections/Collection.h"

// CHECK: Rename the 'IndexedCollection' interface to 'Sequence'?

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Set of items that can be accessed individually by index</summary>
  /// <remarks>
  ///   <para>
  ///     The indexed collection interface is not meant for local or private use within
  ///     your code since the Standard C++ Library contains a host of very well designed
  ///     and powerful container classes that are the envy of other languages far and wide.
  ///   </para>
  ///   <para>
  ///     Use this interface where you might want to expose a collection to users of
  ///     a library without binding yourself to a specific container type. This allows
  ///     you to expose a list of things in a natural and consistent way without
  ///     duplicating Add() and Remove() methods, while keeping complete freedom over
  ///     the actual data structure used to store the items.
  ///   </para>
  ///   <para>
  ///     Java and .NET call this a 'list', but in C++ this name is already taken by
  ///     the standard's linked list implementation.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class IndexedCollection : public Collection<TValue> {

    /// <summary>An index that is never given to any item in the collection</summary>
    public: static const std::size_t InvalidIndex;

    /// <summary>Initializes a new indexed collection</summary>
    protected: IndexedCollection() = default;
    /// <summary>Frees all memory used by the collection</summary>
    public: virtual ~IndexedCollection() = default;

    /// <summary>Determines the index of the specified item in the collection</summary>
    /// <param name="value">Item whose index will be determined</param>
    /// <returns>The index of the specified item</returns>
    public: virtual std::size_t GetIndexOf(const TValue &value) const = 0;

    /// <summary>Retrieves the item at the specified index</summary>
    /// <param name="index">Index of the item that will be retrieved</param>
    /// <returns>The item at the specified index</returns>
    public: virtual const TValue &GetAt(std::size_t index) const = 0;

    /// <summary>Accesses the item at the specified index</summary>
    /// <param name="index">Index of the item that will be accessed</param>
    /// <returns>The item at the specified index</returns>
    public: virtual TValue &GetAt(std::size_t index) = 0;

    /// <summary>Assigns the specified item to the specified index</summary>
    /// <param name="index">Index at which the item will be stored</param>
    /// <param name="value">Item that will be stored at the specified index</param>
    public: virtual void SetAt(std::size_t index, const TValue &value) = 0;

    /// <summary>Inserts the specified item at a specified index</summary>
    /// <param name="index">Index at which the item will be inserted</param>
    /// <param name="value">Item that will be inserted into the collection</param>
    public: virtual void InsertAt(std::size_t index, const TValue &value) = 0;

    /// <summary>Removes the item at the specified index from the collection</summary>
    /// <param name="index">Index at which the item will be removed</param>
    public: virtual void RemoveAt(std::size_t index) = 0;

    //private: IndexedCollection(const IndexedCollection &) = delete;
    //private: IndexedCollection &operator =(const IndexedCollection &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TValue>
  const std::size_t IndexedCollection<TValue>::InvalidIndex = std::size_t(-1);

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_INDEXEDCOLLECTION_H
