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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Set of items that can be accessed sequentially</summary>
  /// <remarks>
  ///   <para>
  ///     The collection interface is not meant for local or private use within your
  ///     code since the Standard C++ Library contains a host of very well designed and
  ///     powerful container classes that are the envy of other languages far and wide.
  ///   </para>
  ///   <para>
  ///     Use this interface where you might want to expose a collection to users of
  ///     a library without binding yourself to a specific container type. This allows
  ///     you to expose a list of things in a natural and consistent way without
  ///     duplicating Add() and Remove() methods, while keeping complete freedom over
  ///     the actual data structure used to store the items.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class Collection {

    #pragma region class Enumerator

    /// <summary>Iterates over the items in the collection</summary>
    public: class Enumerator {

      /// <summary>Frees any memory used by the enumerator</summary>
      public: virtual ~Enumerator() = default;

      /// <summary>Advances to the next item in the collection, if available</summary>
      /// <returns>True if there was a next item, false if the end was reached</returns>
      public: virtual bool Advance() = 0;

      /// <summary>Retrieves the item at the current enumerator position</summary>
      /// <remarks>
      ///   The enumerator starts out on an empty position, so you have to call
      ///   <see cref="Advance" /> as the very first method of a new enumerator
      ///   (and if the collection is empty, that first call will return false).
      /// </remarks>
      public: virtual const TValue &Get() const = 0;

      private: Enumerator(const Enumerator &) = delete;
      private: Enumerator &operator =(const Enumerator &) = delete;

    };

    #pragma endregion // class Enumerator

    /// <summary>Initializes a new collection</summary>
    protected: Collection() = default;
    /// <summary>Frees all memory used by the collection</summary>
    public: virtual ~Collection() = default;

    /// <summary>Adds the specified item to the collection</summary>
    /// <param name="item">Item that will be added to the collection</param>
    public: virtual void Add(const TValue &item) = 0;

    /// <summary>Removes the specified item from the collection</summary>
    /// <param name="item">Item that will be removed from the collection</param>
    /// <returns>True if the item existed in the collection and was removed</returns>
    public: virtual bool Remove(const TValue &item) = 0;

    /// <summary>Removes all items from the collection</summary>
    public: virtual void Clear() = 0;

    /// <summary>Checks if the collection contains the specified item</summary>
    /// <param name="item">Item the collection will be checked for</param>
    /// <returns>True if the collection contain the specified item, false otherwise</returns>
    public: virtual bool Contains(const TValue &item) const = 0;

    /// <summary>Counts the number of items in the collection</summary>
    /// <returns>The number of items the collection contains</returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the collection is empty</summary>
    /// <returns>True if the collection is empty</returns>
    public: virtual bool IsEmpty() const = 0;

    //private: Collection(const Collection &) = delete;
    //private: Collection &operator =(const Collection &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_COLLECTION_H
