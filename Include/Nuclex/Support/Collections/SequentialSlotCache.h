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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H
#define NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Collections/Cache.h" // for Cache

#include <memory> // for std::unique_ptr

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Caches items that can be addressed through a linear, zero-based index</summary>
  /// <typeparam name="TKey">Type of the key the cache uses, must be an integer</typeparam>
  /// <typeparam name="TValue">Type of values that are stored in the cache</typeparam>
  template<typename TKey, typename TValue>
  class SequentialSlotCache : public Cache<TKey, TValue> {

    /// <summary>Initializes a new slot cache with the specified number of slots</summary>
    /// <param name="slotCount">Number of slots the cache will provide</param>
    public: SequentialSlotCache(std::size_t slotCount);

    /// <summary>Frees all memory used by the sequential slot cache</summary>
    public: virtual ~SequentialSlotCache() = default;

    /// <summary>Stores a value in the map</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key in the map</param>
    /// <returns>
    ///   True if the key did not exist before and was inserted,
    ///   false if the key already existed and its value was replaced.
    /// </returns>
    public: bool Insert(const TKey &key, const TValue &value) override;

    /// <summary>Stores a value in the map if it doesn't exist yet</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key in the map</param>
    /// <returns>
    ///   True if the key did not exist before and was inserted,
    ///   false if the key already existed and left unchanged
    /// </returns>
    public: bool TryInsert(const TKey &key, const TValue &value) override;

    /// <summary>Returns the value of the specified element in the map</summary>
    /// <param name="key">Key of the element that will be looked up</param>
    public: const TValue &Get(const TKey &key) const override;
    
    /// <summary>Tries to look up an element in the map</summary>
    /// <param name="key">Key of the element that will be looked up</param>
    /// <param name="value">Will receive the value if the element was found</param>
    /// <returns>
    ///   True if an element was returned, false if the key didn't exist
    /// </returns>
    public: bool TryGet(const TKey &key, TValue &value) const override;

    /// <summary>Tries to take an element from the map (removing it)</summary>
    /// <param name="key">Key of the element that will be taken from the map</param>
    /// <param name="value">Will receive the value taken from the map</param>
    /// <returns>
    ///   True if an element was found and removed from the map, false if the key didn't exist
    /// </returns>
    public: bool TryTake(const TKey &key, TValue &value) override;

    // CHECK: Could provide a TryTake() with output method to allow for move semantics.
    //public: virtual bool TryTake(const TKey &key, const std::function<TValue &&> &valueCallback) = 0;

    /// <summary>Removes the specified element from the map if it exists</summary>
    /// <param name="key">Key of the element that will be removed if present</param>
    /// <returns>True if the element was found and removed, false otherwise</returns>
    public: bool TryRemove(const TKey &key) override;

    /// <summary>Removes all items from the map</summary>
    public: void Clear() override;

    /// <summary>
    ///   Evicts items from the cache until at most <see cref="itemCount" /> items remain
    /// </summary>
    /// <param name="itemCount">Maximum number of items that will be left behind</param>
    public: void EvictDownTo(std::size_t itemCount) override;

    /// <summary>Evicts items from the cache until reaching a user-defined criterion</summary>
    /// <param name="policyCallback">Callback that decides whether to evict an entry</param>
    public: void EvictWhere(
      const Events::Delegate<bool(const TValue &)> &policyCallback
    ) override;

    /// <summary>Counts the numebr of elements currently in the map</summary>
    /// <returns>
    ///   The approximate number of elements that had been in the map during the call
    /// </returns>
    public: std::size_t Count() const override;

    /// <summary>Checks if the map is empty</summary>
    /// <returns>True if the map had been empty during the call</returns>
    public: bool IsEmpty() const override;

    //private: Cache(const Cache &) = delete;
    //private: Cache &operator =(const Cache &) = delete;

    #pragma region struct SlotState

    /// <summary>Status of a slot, including its place in the MRU list</summary>
    /// <remarks>
    ///   The slot index is not stored separately as it can easily be obtained via pointer
    ///   arithmetic (since the states are all stored in a single linear array)
    /// </remarks>
    private: struct SlotState {

      /// <summary>Whether this slot is occupied or empty</summary>
      public: bool IsOccupied;
      /// <summary>Link to the previous element in the MRU doubly linked list</summary>
      public: SlotState *LessRecentlyUsed;
      /// <summary>Link to the next element in the MRU doubly linked list</summary>
      public: SlotState *MoreRecentlyUsed;

    };

    #pragma endregion // struct SlotState

    /// <summary>Number of slots currently filled in the cache</summary>
    private: std::size_t count;
    /// <summary>Keeps track of the state of each individual slot</summary>
    private: std::unique_ptr<SlotState[]> states;
    /// <summary>Values stored in each of the slots</summary>
    private: std::unique_ptr<TValue[]> values;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  SequentialSlotCache<TKey, TValue>::SequentialSlotCache(std::size_t slotCount) :
    count(0),
    states(new SlotState[slotCount]),
    values(new TValue[slotCount]) {}

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::Insert(const TKey &key, const TValue &value) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryInsert(const TKey &key, const TValue &value) {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  const TValue &SequentialSlotCache<TKey, TValue>::Get(const TKey &key) const {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryGet(const TKey &key, TValue &value) const {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryTake(const TKey &key, TValue &value) {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryRemove(const TKey &key) {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::Clear() {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::EvictDownTo(std::size_t itemCount) {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::EvictWhere(
    const Events::Delegate<bool(const TValue &)> &policyCallback
  ) {
    throw -1;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  std::size_t SequentialSlotCache<TKey, TValue>::Count() const {
    return this->count;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::IsEmpty() const {
    return (this->count == 0);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H
