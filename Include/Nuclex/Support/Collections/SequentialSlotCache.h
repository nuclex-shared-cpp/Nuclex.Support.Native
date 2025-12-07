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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H
#define NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Collections/Cache.h" // for Cache
#include "Nuclex/Support/Errors/KeyNotFoundError.h" // for KeyNotFoundError

#include <cstddef> // for std::byte

namespace Nuclex::Support::Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Caches items that can be addressed through a linear, zero-based index</summary>
  /// <typeparam name="TKey">Type of the key the cache uses, must be an integer</typeparam>
  /// <typeparam name="TValue">Type of values that are stored in the cache</typeparam>
  /// <remarks>
  ///   <para>
  ///     This type of cache is ideal if you have a fixed number of items (for example,
  ///     files in a directory or frames in a video) that can be addressed through a simple
  ///     integer index.
  ///   </para>
  ///   <para>
  ///     It keeps these items in a linear array (wherein &quot;slots&quot; can be either
  ///     occupied or empty, just like <see cref="std.vector" />, but also maintains
  ///     a doubly-linked MRU list between those items, preventing memory fragmentation from
  ///     micro allocations, enabling cache-friendly searches through linear memory whilst
  ///     offering cheap MRU functionality like evict, bring to top, get oldest).
  ///   </para>
  /// </remarks>
  template<typename TKey, typename TValue>
  class SequentialSlotCache : public Cache<TKey, TValue> {

    /// <summary>Initializes a new slot cache with the specified number of slots</summary>
    /// <param name="slotCount">Number of slots the cache will provide</param>
    public: SequentialSlotCache(std::size_t slotCount);

    /// <summary>Frees all memory used by the sequential slot cache</summary>
    public: virtual ~SequentialSlotCache();

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

    /// <summary>Evicts items from the cache matching a user-defined criterion</summary>
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

    /// <summary>
    ///   Moves the specified slot state to the top of the most recently used list
    /// </summary>
    /// <param name="slotState">Slot that will become the most recently used</param>
    private: void makeMostRecentlyUsed(SlotState &slotState) const; // <- in mutable state

    /// <summary>Integrates the specified slot state into the most recently used list</summary>
    /// <param name="slotState">Slot state that will be integratedf into the MRU list</param>
    private: void linkMostRecentlyUsed(SlotState &slotState) const; // <- in mutable state

    /// <summary>Removes the specified slot state from the most recently used list</summary>
    /// <param name="slotState">Slot state that will be removed from the MRU list</param>
    private: void unlinkMostRecentlyUsed(SlotState &slotState) const; // <- in mutable state

    /// <summary>
    ///   Calculates the amount of memory needed for buffer holding both the slot states
    ///   and the values that can be stored in the cache
    /// </summary>
    /// <param name="slotCount">Number of slots for which the memory is calculated</param>
    /// <returns>The required memory to store slots and values with alignment</returns>
    private: constexpr static std::size_t getRequiredMemory(std::size_t slotCount) {
      return (
        (sizeof(SlotState[2]) * slotCount / 2) +
        (sizeof(TValue[2]) * slotCount / 2) +
        (alignof(SlotState) - 1) + // for initial alignment padding if needed
        (
          alignof(TValue) > alignof(SlotState) ?
            (alignof(TValue) - alignof(SlotState)) : 0
        )
      );
    }

    /// <summary>Number of slots currently filled in the cache</summary>
    private: std::size_t count;
    /// <summary>Memory allocated to store the slot states and values</summary>
    private: std::byte *memory;
    /// <summary>Values stored in each of the slots</summary>
    private: TValue *values;
    /// <summary>Keeps track of the state of each individual slot</summary>
    private: mutable SlotState *states;
    /// <summary>Pointer to the state of the most recently used slot</summary>
    private: mutable SlotState *mostRecentlyUsed;
    /// <summary>Pointer to the state of the least recently used slot</summary>
    private: mutable SlotState *leastRecentlyUsed;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  SequentialSlotCache<TKey, TValue>::SequentialSlotCache(std::size_t slotCount) :
    count(0),
    memory(new std::byte[getRequiredMemory(slotCount)]),
    values(),
    states(),
    mostRecentlyUsed(nullptr),
    leastRecentlyUsed(nullptr) {

    // Calculate the aligned memory address where slot states will be stored
    {
      std::size_t misalignment = (
        reinterpret_cast<std::uintptr_t>(this->memory) % alignof(SlotState)
      );
      if(misalignment > 0) {
        this->states = reinterpret_cast<SlotState *>(
          this->memory + alignof(SlotState) - misalignment
        );
      } else {
        this->states = reinterpret_cast<SlotState *>(this->memory);
      }
    }

    // Place the values directly behind the slot state array, with alignment padding
    // if the end of the slot state array doesn't meet the value's alignment needs.
    {
      std::uintptr_t valueMemory = (
        reinterpret_cast<std::uintptr_t>(this->states) +
        (sizeof(SlotState[2]) * slotCount / 2)
      );
      std::size_t misalignment = valueMemory % alignof(TValue);

      if(misalignment > 0) {
        this->values = reinterpret_cast<TValue *>(
          valueMemory + alignof(TValue) - misalignment
        );
      } else {
        this->values = reinterpret_cast<TValue *>(valueMemory);
      }
    }

    // Initialize all IsOccupied values to false so we don't accidentally try to destroy
    // values that weren't present (but where the uninitialized memory in which we built
    // the slot state array happened to have the appropriate bits set to make IsOccupied true).
    for(std::size_t index = 0; index < slotCount; ++index) {
      this->states[index].IsOccupied = false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  SequentialSlotCache<TKey, TValue>::~SequentialSlotCache() {
    Clear();
    delete[] this->memory;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::Insert(const TKey &key, const TValue &value) {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      TValue *address = this->values + key;
      address->~TValue();
      new(address) TValue(value);
      makeMostRecentlyUsed(state);
      return false;
    } else {
      new(this->values + key) TValue(value);
      state.IsOccupied = true;
      ++this->count;
      linkMostRecentlyUsed(state);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryInsert(const TKey &key, const TValue &value) {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      return false;
    } else {
      new(this->values + key) TValue(value);
      state.IsOccupied = true;
      ++this->count;
      linkMostRecentlyUsed(state);
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  const TValue &SequentialSlotCache<TKey, TValue>::Get(const TKey &key) const {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      makeMostRecentlyUsed(state);
      return this->values[key];
    } else {
      throw Errors::KeyNotFoundError(
        reinterpret_cast<const char *>(u8"Requested cache slot is empty")
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryGet(const TKey &key, TValue &value) const {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      makeMostRecentlyUsed(state);
      value = this->values[key];
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryTake(const TKey &key, TValue &value) {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      TValue *address = this->values + key;
      value = std::move(*address);
      address->~TValue();
      state.IsOccupied = false;
      unlinkMostRecentlyUsed(state);
      --this->count;
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool SequentialSlotCache<TKey, TValue>::TryRemove(const TKey &key) {
    SlotState &state = this->states[key];
    if(state.IsOccupied) {
      TValue *address = this->values + key;
      address->~TValue();
      state.IsOccupied = false;
      unlinkMostRecentlyUsed(state);
      --this->count;
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::Clear() {
    SlotState *current = this->mostRecentlyUsed;
    while(current != nullptr) {
      std::ptrdiff_t index = current - this->states;

      this->values[index].~TValue();
      current->IsOccupied = false;

      current = current->LessRecentlyUsed;
    }

    this->count = 0;
    this->leastRecentlyUsed = this->mostRecentlyUsed = nullptr;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::EvictDownTo(std::size_t itemCount) {
    SlotState *current = this->leastRecentlyUsed;
    while(current != nullptr) {
      if(itemCount >= this->count) {
        break;
      }

      std::ptrdiff_t index = current - this->states;
      this->values[index].~TValue();
      current->IsOccupied = false;
      --this->count;

      current = current->MoreRecentlyUsed;
    }

    if(current == nullptr) {
      this->leastRecentlyUsed = this->mostRecentlyUsed = nullptr;
    } else {
      current->LessRecentlyUsed = nullptr;
      this->leastRecentlyUsed = current;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::EvictWhere(
    const Events::Delegate<bool(const TValue &)> &policyCallback
  ) {
    SlotState *current = this->leastRecentlyUsed;
    while(current != nullptr) {
      std::ptrdiff_t index = current - this->states;

      bool evict = policyCallback(this->values[index]);
      if(evict) {
        unlinkMostRecentlyUsed(*current);
        this->values[index].~TValue();
        current->IsOccupied = false;
        --this->count;
      }

      current = current->MoreRecentlyUsed;
    }
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

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::makeMostRecentlyUsed(SlotState &slotState) const {

    // Only do something if the slot in question isn't already the most recent used one
    if(slotState.MoreRecentlyUsed != nullptr) {
      slotState.MoreRecentlyUsed->LessRecentlyUsed = slotState.LessRecentlyUsed;
      if(slotState.LessRecentlyUsed == nullptr) {
        this->leastRecentlyUsed = slotState.MoreRecentlyUsed;
      } else {
        slotState.LessRecentlyUsed->MoreRecentlyUsed = slotState.MoreRecentlyUsed;
      }

      slotState.LessRecentlyUsed = this->mostRecentlyUsed;
      slotState.MoreRecentlyUsed = nullptr;
      this->mostRecentlyUsed->MoreRecentlyUsed = &slotState;
      this->mostRecentlyUsed = &slotState;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::linkMostRecentlyUsed(SlotState &slotState) const {
    if(this->mostRecentlyUsed == nullptr) {
      slotState.LessRecentlyUsed = slotState.MoreRecentlyUsed = nullptr;
      this->leastRecentlyUsed = this->mostRecentlyUsed = &slotState;
    } else {
      slotState.LessRecentlyUsed = this->mostRecentlyUsed;
      slotState.MoreRecentlyUsed = nullptr;
      this->mostRecentlyUsed->MoreRecentlyUsed = &slotState;
      this->mostRecentlyUsed = &slotState;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void SequentialSlotCache<TKey, TValue>::unlinkMostRecentlyUsed(SlotState &slotState) const {
    if(slotState.LessRecentlyUsed == nullptr) {
      this->leastRecentlyUsed = slotState.MoreRecentlyUsed;
    } else {
      slotState.LessRecentlyUsed->MoreRecentlyUsed = slotState.MoreRecentlyUsed;
    }
    
    if(slotState.MoreRecentlyUsed == nullptr) {
      this->mostRecentlyUsed = slotState.LessRecentlyUsed;
    } else {
      slotState.MoreRecentlyUsed->LessRecentlyUsed = slotState.LessRecentlyUsed;
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_SEQUENTIALSLOTCACHE_H
