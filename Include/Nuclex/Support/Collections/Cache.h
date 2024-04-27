#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CACHE_H
#define NUCLEX_SUPPORT_COLLECTIONS_CACHE_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Collections/Map.h" // for Map
#include "Nuclex/Support/Events/Delegate.h" // for Delegate

namespace Nuclex { namespace Support { namespace Collections {

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

    /// <summary>Evicts items from the cache until reaching a user-defined criterion</summary>
    /// <param name="policyCallback">Callback that decides whether to evict an entry</param>
    public: virtual void EvictWhere(
      const Events::Delegate<bool(const TValue &)> &policyCallback
    ) = 0;

    //private: Cache(const Cache &) = delete;
    //private: Cache &operator =(const Cache &) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CACHE_H
