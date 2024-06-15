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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Set that can safely be used from multiple threads</summary>
  /// <typeparam name="TKey">Type of the key the set will keep track of</typeparam>
  template<typename TKey>
  class ConcurrentSet {

    /// <summary>Destroys the concurrent set</summary>
    public: virtual ~ConcurrentSet() = default;

    /// <summary>Tries to insert a key into the set in a thread-safe manner</summary>
    /// <param name="key">Key that will be inserted into the set</param>
    /// <returns>True if the key was inserted, false if the key already existed</returns>
    public: virtual bool TryInsert(const TKey &key) = 0;

    /// <summary>Tries to remove a key from the set</summary>
    /// <param name="key">Key that will be removed from the set</param>
    /// <returns>
    ///   True if the key was removed from the set, false if the key didn't exist (anymore?)
    /// </returns>
    public: virtual bool TryRemove(const TKey &key) = 0;

    /// <summary>Counts the numebr of keys currently in the set</summary>
    /// <returns>
    ///   The approximate number of keys that had been in the set during the call
    /// </returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the set is empty</summary>
    /// <returns>True if the set had been empty during the call</returns>
    public: virtual bool IsEmpty() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
