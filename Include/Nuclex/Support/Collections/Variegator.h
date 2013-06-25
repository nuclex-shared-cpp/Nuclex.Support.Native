#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_VARIEGATOR_H
#define NUCLEX_SUPPORT_COLLECTIONS_VARIEGATOR_H

#include <cstddef>
#include <random>

#include <vector>
#include <set>
#include <map>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Randomly selects between different options, trying to avoid repetition</summary>
  /// <typeparam name="TKey">Type of keys through which values can be looked up</typeparam>
  /// <typeparam name="TValue">Type of values provided by the variegator</typeparam>
  /// <typeparam name="TValueAllocator">Allocator used to reserve memory for values</typeparam>
  /// <remarks>
  ///   <para>
  ///     This class is useful wherever randomness is involved in a game: picking random
  ///     actions for an NPC to execute, selecting different songs to play, displaying
  ///     different dialogue and more.
  ///   </para>
  ///   <para>
  ///     In principle, it works like a multimap, associating keys with a number of values
  ///     and allowing you to look up a values by their keys. Unlike a multimap, it will
  ///     avoid handing out a previously provided value again.
  ///   </para>
  ///   <para>
  ///     A typical usage would be to set up a mapping between situations and dialogue lines.
  ///     Upon calling <see cref="Get" /> with the situation 'detected-player-stealing',
  ///     the variegator would return a random (but not recently used) value which in this case
  ///     might contain a commentary an NPC might make upon encountering that situation.
  ///     Other NPCs requesting dialogue lines for the same situation would receive different
  ///     random commentary for as long as long as available data allows.
  ///   </para>
  /// </remarks>
  template<typename TKey, typename TValue, typename TValueAllocator = std::allocator<TValue>>
  class Variegator {

    /// <summary>Initializes a new variegator</summary>
    /// <param name="historyLength">
    ///   How far into the past the variegator will look to avoid repetition
    /// </param>
    public: Variegator(std::size_t historyLength = 64) :
      historyLength(historyLength),
      historyFull(false),
      historyTailIndex(0),
      history(this->valueAllocator.allocate(historyLength)) {}

    /// <summary>Destroys the variegator and reclaims all memory</summary>
    public: ~Variegator() {
      freeHistory();
      this->valueAllocator.deallocate(this->history, this->historyLength);
    }

    /// <summary>Removes all entries from the variegator</summary>
    /// <remarks>
    ///   This is mainly useful if you are storing smart pointers to the values and need
    ///   reclaim memory.
    /// </remarks>
    public: void Clear() {
      this->values.clear();
      freeHistory();

      this->historyFull = false;
      this->historyTailIndex = 0;
    }

    /// <summary>Checks whether the variegator is empty</summary>
    /// <returns>True if there are no entries in the variegator</returns>
    public: bool IsEmpty() const {
      return this->values.empty();
    }

    /// <summary>
    ///   Insert a new value that can be returned when requesting the specified key
    /// </summary>
    /// <param name="key">Key of the value that will be inserted<param>
    /// <param name="value">Value that will be inserted under the provided key</param>
    public: void Insert(const TKey &key, const TValue &value) {
      this->values.insert(ValueMap::value_type(key, value));
    }

    /// <summary>Retrieves a random value associated with the specified key</summary>
    /// <param name="key">For for which a value will be looked up</param>
    /// <returns>A random value associated with the specified key</returns>
    public: TValue Get(const TKey &key) const {
      return Get(&key, std::size_t(1));
    }

    /// <summary>Retrieves a random value associated with one of the specified keys</summary>
    /// <param name="first">First key in a list of keys that will be considered</param>
    /// <param name="count">Number of keys following the first key in the list</param>
    /// <remarks>
    ///   In many cases, you have generic situations (such as 'detected-player-stealing',
    ///   'observed-hostile-action') and specified situations (such as
    ///   'detected-player-stealing-from-beggar', 'observed-hostile-action-on-cop')
    ///   where a values from both pools should be considered. This method allows you
    ///   to specify any number of keys, creating a greater set of values the variegator
    ///   can pick between.
    /// </remarks>
    public: template<typename TInputIterator> TValue Get(
      TInputIterator first, std::size_t count = 1
    ) const {
      std::set<TValue> candidates;

      while(count > 0) {
        std::pair<ValueMap::const_iterator, ValueMap::const_iterator> valueRange =
          this->values.equal_range(*first);

        // Add all candidate values into our set. Could do this via std::copy() with
        // std::inserter(), but it yields the same amount of code and adds a header dependency
        while(valueRange.first != valueRange.second) {
          candidates.insert(valueRange.first->second);
          ++valueRange.first;
        }

        ++first;
        --count;
      }

      TValue result = destructivePickCandidateValue(candidates);
      addRecentlyUsedValue(result);
      return result;
    }

    /// <summary>Retrieves a random value associated with one of the specified keys</summary>
    /// <param name="first">First key in a list of keys that will be considered</param>
    /// <param name="onePastLast">
    ///   Iterator past the last in the list of keys that will be considered
    /// </param>
    /// <remarks>
    ///   In many cases, you have generic situations (such as 'detected-player-stealing',
    ///   'observed-hostile-action') and specified situations (such as
    ///   'detected-player-stealing-from-beggar', 'observed-hostile-action-on-cop')
    ///   where a values from both pools should be considered. This method allows you
    ///   to specify any number of keys, creating a greater set of values the variegator
    ///   can pick between.
    /// </remarks>
    public: template<typename TInputIterator> TValue Get(
      TInputIterator first, TInputIterator onePastLast
    ) const {
      std::set<TValue> candidates;

      while(first != onePastLast) {
        std::pair<ValueMap::const_iterator, ValueMap::const_iterator> valueRange =
          this->values.equal_range(*first);

        // Add all candidate values into our set. Could do this via std::copy() with
        // std::inserter(), but it yields the same amount of code and adds a header dependency
        while(valueRange.first != valueRange.second) {
          candidates.insert(valueRange.first->second);
          ++valueRange.first;
        }

        ++first;
      }

      TValue result = destructivePickCandidateValue(candidates);
      addRecentlyUsedValue(result);
      return result;
    }

    /// <summary>Picks amongst the values in a set</summary>
    /// <param name="candidates">
    ///   Set containing the candidats values to consider. Will be destroyed.
    /// </param>
    /// <returns>The least recently used candidate value or a random one</returns>
    private: TValue destructivePickCandidateValue(std::set<TValue> &candidates) const {
      removeRecentlyUsedValues(candidates);

      switch(candidates.size()) {
        case 0: {
          throw std::runtime_error("No values mapped to this key");
        }
        case 1: {
          return *candidates.begin();
        }
        default: {
          std::uniform_int_distribution<std::size_t> distributor(0, candidates.size() - 1);
          std::size_t index = distributor(this->randomNumberGenerator);
          
          std::set<TValue>::const_iterator iterator = candidates.begin();
          while(index > 0) {
            ++iterator;
            --index;
          }

          return *iterator;
        }
      }
    }

    /// <summary>Adds a recently used value to the history</summary>
    /// <param name="value">Value that will be added to the history</param>
    private: void addRecentlyUsedValue(const TValue &value) const {
      if(this->historyTailIndex == this->historyLength) {
        this->historyFull = true;
        this->history[0] = value;
        this->historyTailIndex = 1;
      } else if(this->historyFull) {
        this->history[this->historyTailIndex] = value;
        ++this->historyTailIndex;
      } else {
        this->valueAllocator.construct(this->history + this->historyTailIndex, value);
        ++this->historyTailIndex;
      }
    }

    /// <summary>Removes all values that are in the recent use list from a set</summary>
    /// <param name="candidates">Set from which recently used values are removed</param>
    /// <remarks>
    ///   Stops removing values when there's only 1 value left in the set
    /// </remarks>
    private: void removeRecentlyUsedValues(std::set<TValue> &candidates) const {
      if(candidates.size() <= 1) {
        return;
      }

      if(this->historyFull) { // History buffer has wrapped around
        std::size_t index = this->historyTailIndex;
        while(index > 0) {
          --index;
          if(candidates.erase(this->history[index])) {
            if(candidates.size() <= 1) {
              return;
            }
          }
        }
        index = this->historyLength;
        while(index > this->historyTailIndex) {
          --index;
          if(candidates.erase(this->history[index])) {
            if(candidates.size() <= 1) {
              return;
            }
          }
        }
      } else { // History buffer was not full yet
        std::size_t index = this->historyTailIndex;
        while(index > 0) {
          --index;
          if(candidates.erase(this->history[index])) {
            if(candidates.size() <= 1) {
              return;
            }
          }
        }
      }
    }

    /// <summary>Frees all memory used by the individual history entries</summary>
    /// <remarks>
    ///   The history array itself is kept alive and the tail index + full flag will
    ///   not be reset.
    /// </remarks>
    private: void freeHistory() {
      if(this->historyFull) { // History buffer has wrapped around
        std::size_t index = this->historyTailIndex;
        while(index > 0) {
          --index;
          this->valueAllocator.destroy(this->history + index);
        }
        index = this->historyLength;
        while(index > this->historyTailIndex) {
          --index;
          this->valueAllocator.destroy(this->history + index);
        }
      } else { // History buffer was not full yet
        std::size_t index = this->historyTailIndex;
        while(index > 0) {
          --index;
          this->valueAllocator.destroy(this->history + index);
        }
      }
    }

    private: Variegator(const Variegator &);
    private: Variegator &operator =(const Variegator &);

    /// <summary>Map by which potential values can be looked up via their key</summary>
    private: typedef std::multimap<TKey, TValue> ValueMap;

    /// <summary>Stores the entries the variegator can select from by their keys</summary>
    private: ValueMap values;

    /// <summary>Random number generator that will be used to pick random values</summary>
    private: mutable std::default_random_engine randomNumberGenerator;
    /// <summary>Used to allocate values in the recently used list</summary>
    private: mutable TValueAllocator valueAllocator;
    /// <summary>Number of entries in the recently used list</summary>
    private: std::size_t historyLength;

    /// <summary>Array containing the most recently provided values</summary>
    private: mutable TValue *history;
    /// <summary>Index of the tail in the recently used value array</summary>
    private: mutable std::size_t historyTailIndex;
    /// <summary>Whether the recently used value history is at capacity</summary>
    private: mutable bool historyFull;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_VARIEGATOR_H
