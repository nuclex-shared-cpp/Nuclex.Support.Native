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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_KEYEDARRAYCACHE_H
#define NUCLEX_SUPPORT_COLLECTIONS_KEYEDARRAYCACHE_H

#include "Nuclex/Support/Config.h"

#include "Nuclex/Support/Collections/MultiCache.h" // for MultiCache
#include "Nuclex/Support/Errors/KeyNotFoundError.h" // for KeyNotFoundError

#include <cstddef> // for std::byte
#include <optional> // for std::optional<>

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
  class KeyedArrayCache : public MultiCache<TKey, TValue> {

    /// <summary>Initializes a new array cache with the specified size</summary>
    /// <param name="capacity">Maximum number of entries the cache may hold</param>
    public: KeyedArrayCache(std::size_t capacity);

    /// <summary>Frees all memory used by the array cache</summary>
    public: virtual ~KeyedArrayCache();

    /// <summary>Stores a value in the cache</summary>
    /// <param name="key">Key under which the value can be looked up later</param>
    /// <param name="value">Value that will be stored under its key</param>
    /// <returns>True in all cases</returns>
    public: bool Insert(const TKey &key, const TValue &value) override;

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
    public: std::size_t TryRemove(const TKey &key) override;

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
    private: struct SlotState {

      /// <summary>Initializes a new slot state</summary>
      public: SlotState() : Key() {} // leave LessRecentlyUsed and MoreRecentlyUsed alone

      /// <summary>Whether this slot is occupied and if so, its key</summary>
      public: std::optional<TKey> Key;
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

    /// <summary>Number of entry currently stored in the cache</summary>
    private: std::size_t count;
    /// <summary>Number of entries the cache can hold</summary>
    private: std::size_t capacity;
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
  KeyedArrayCache<TKey, TValue>::KeyedArrayCache(std::size_t capacity) :
    count(0),
    capacity(capacity),
    memory(new std::byte[getRequiredMemory(capacity)]),
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
        (sizeof(SlotState[2]) * capacity / 2)
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
    for(std::size_t index = 0; index < capacity; ++index) {
      new(&this->states[index]) SlotState();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  KeyedArrayCache<TKey, TValue>::~KeyedArrayCache() {
    Clear();
    delete[] this->memory;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool KeyedArrayCache<TKey, TValue>::Insert(const TKey &key, const TValue &value) {

    // If there is still space left in the cache, do not overwrite an existing
    // entry but find a space for a new entry to be inserted.
    if(this->count < this->capacity) {

      // Shortcut: most caches will be constructed empty, fill up and stay full,
      // evicting the oldest items as needed. Thus, it is a good guess to check
      // the array index that matches the current item count first.
      if(!this->states[this->count].Key.has_value()) {
        new(this->values + this->count) TValue(value);
        this->states[this->count].Key = key;

        linkMostRecentlyUsed(this->states[this->count]);
        ++this->count;
        return true;
      }

      // The array index matching the item count was not empty, probably because
      // items were evicted manually, so now we have to scan the entire array for
      // a free index.
      for(std::size_t index = 0; index < this->capacity; ++index) {
        if(!this->states[index].Key.has_value()) {
          new(this->values + index) TValue(value);
          this->states[index].Key = key;

          ++this->count;
          linkMostRecentlyUsed(this->states[index]);
          return true;
        }
      }

      // If this point is reached, we failed as basic bookkeeping and it's a bug!
      assert(!u8"Item count says entries should be available, but cache is full.");

    }

    // There was no free array index in the cache, so we'll directly pick the least recently
    // used entry and overwrite it with the new one.
    {
      std::ptrdiff_t index = this->leastRecentlyUsed - this->states;
      //std::size_t index = static_cast<std::uintptr_t>(this->leastRecentlyUsed - this->states);
      //index /= sizeof(SlotState[2]) / 2;

      this->leastRecentlyUsed->Key = key;

      TValue *address = this->values + index;
      address->~TValue();
      new(address) TValue(value);
      makeMostRecentlyUsed(*this->leastRecentlyUsed);
    }

    return true; // it was inserted (inheriting Map<K, T> requires this pointless result)
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  const TValue &KeyedArrayCache<TKey, TValue>::Get(const TKey &key) const {
    for(std::size_t index = 0; index < this->capacity; ++index) {
      if(this->states[index].Key == key) { // empty Keys will not compare as equal
        makeMostRecentlyUsed(this->states[index]);
        return this->values[index];
      }
    }
    throw Errors::KeyNotFoundError(
      reinterpret_cast<const char *>(u8"Requested key not found in cache")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool KeyedArrayCache<TKey, TValue>::TryGet(const TKey &key, TValue &value) const {
    for(std::size_t index = 0; index < this->capacity; ++index) {
      if(this->states[index].Key == key) { // empty Keys will not compare as equal
        value = this->values[index];
        makeMostRecentlyUsed(this->states[index]);
        return true;
      }
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool KeyedArrayCache<TKey, TValue>::TryTake(const TKey &key, TValue &value) {
    for(std::size_t index = 0; index < this->capacity; ++index) {
      if(this->states[index].Key == key) { // empty Keys will not compare as equal
        TValue *address = this->values + index;
        value = std::move(*address);
        address->~TValue();

        this->states[index].Key.reset(); // = std::optional<TKey>();
        --this->count;
        unlinkMostRecentlyUsed(this->states[index]);
        return true;
      }
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  std::size_t KeyedArrayCache<TKey, TValue>::TryRemove(const TKey &key) {
    std::size_t removedElementCount = 0;

    for(std::size_t index = 0; index < this->capacity; ++index) {
      if(this->states[index].Key == key) { // empty Keys will not compare as equal
        TValue *address = this->values + index;
        address->~TValue();

        this->states[index].Key.reset(); // = std::optional<TKey>();
        --this->count;
        unlinkMostRecentlyUsed(this->states[index]);
        ++removedElementCount;
      }
    }

    return removedElementCount;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void KeyedArrayCache<TKey, TValue>::Clear() {
    SlotState *current = this->mostRecentlyUsed;
    while(current != nullptr) {
      std::ptrdiff_t index = current - this->states;

      this->values[index].~TValue();
      current->Key.reset();

      current = current->LessRecentlyUsed;
    }

    this->count = 0;
    this->leastRecentlyUsed = this->mostRecentlyUsed = nullptr;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void KeyedArrayCache<TKey, TValue>::EvictDownTo(std::size_t itemCount) {
    SlotState *current = this->leastRecentlyUsed;
    while(current != nullptr) {
      if(itemCount >= this->count) {
        break;
      }

      std::ptrdiff_t index = current - this->states;
      this->values[index].~TValue();
      current->Key.reset();
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
  void KeyedArrayCache<TKey, TValue>::EvictWhere(
    const Events::Delegate<bool(const TValue &)> &policyCallback
  ) {
    SlotState *current = this->leastRecentlyUsed;
    while(current != nullptr) {
      std::ptrdiff_t index = current - this->states;

      bool evict = policyCallback(this->values[index]);
      if(evict) {
        unlinkMostRecentlyUsed(*current);
        this->values[index].~TValue();
        current->Key.reset();
        --this->count;
      }

      current = current->MoreRecentlyUsed;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  std::size_t KeyedArrayCache<TKey, TValue>::Count() const {
    return this->count;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  bool KeyedArrayCache<TKey, TValue>::IsEmpty() const {
    return (this->count == 0);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TKey, typename TValue>
  void KeyedArrayCache<TKey, TValue>::makeMostRecentlyUsed(SlotState &slotState) const {

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
  void KeyedArrayCache<TKey, TValue>::linkMostRecentlyUsed(SlotState &slotState) const {
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
  void KeyedArrayCache<TKey, TValue>::unlinkMostRecentlyUsed(SlotState &slotState) const {
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

#endif // NUCLEX_SUPPORT_COLLECTIONS_KEYEDARRAYCACHE_H
