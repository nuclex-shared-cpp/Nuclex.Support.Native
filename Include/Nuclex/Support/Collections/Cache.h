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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CACHE_H
#define NUCLEX_SUPPORT_COLLECTIONS_CACHE_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Collections/Map.h" // for Map
#include "Nuclex/Support/Events/Delegate.h" // for Delegate

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Special type of map that is able to evict its least recently used items</summary>
  /// <typeparam name="TKey">Type of the key the cache uses</typeparam>
  /// <typeparam name="TValue">Type of values that are stored in the cache</typeparam>
  template<typename TKey, typename TValue>
  class Cache : public Map<TKey, TValue> {

    /// <summary>Frees all memory used by the cache</summary>
    public: virtual ~Cache() = default;

    /// <summary>
    ///   Evicts items from the cache until at most <see cref="itemCount" /> items remain
    /// </summary>
    /// <param name="itemCount">Maximum number of items that will be left behind</param>
    public: virtual void EvictDownTo(std::size_t itemCount) = 0;

    /// <summary>Evicts items from the cache that fit a user-defined criterion</summary>
    /// <param name="policyCallback">Callback that decides whether to evict an entry</param>
    public: virtual void EvictWhere(
      const Events::Delegate<bool(const TValue &)> &policyCallback
    ) = 0;

    //private: Cache(const Cache &) = delete;
    //private: Cache &operator =(const Cache &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CACHE_H
