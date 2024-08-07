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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t

// Known implementations of lock-free collections for reference:
//
// Libraries of Lock-Free data structures:
// https://github.com/mpoeter/xenium
// https://liblfds.org/ (<-- Public Domain!)
// https://github.com/khizmax/libcds
//
// Interesting design advice on Moody Camel's blog:
// https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++.htm
// https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++.htm
//
// Intel's implementation (curiously not that good in benchmarks):
// https://github.com/oneapi-src/oneTBB (Intel TBB under its new name)
// https://github.com/oneapi-src/oneTBB/blob/master/include/oneapi/tbb/concurrent_queue.h
//
// Microsoft also ships a non-portable concurrent quue with Visual C++:
// https://learn.microsoft.com/en-us/cpp/parallel/concrt/reference/concurrent-queue-class
//
// "Battle Tested" implementation:
// https://github.com/rigtorp/awesome-lockfree
// https://github.com/rigtorp/MPMCQueue
//
// Moody Camel's implementation (I recommend this one):
// https://github.com/cameron314/concurrentqueue
//

// DESIGN: I think this design isn't optimal.
//   A multi-multi collection could be a stand-in for a single-single collection,
//   but with this design, they're mutually exclusive. It might be better to
//   inherit one from the other, so a ConcurrentCollection<SingleSingle> becomes
//   the base class for a ConcurrentCollection<SingleMulti> which then is the base
//   class to ConcurrentCollection<MultiMulti>.
//
// Can this be represented differently, with traits of concepts?

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>How a concurrent collection is being accessed</summary>
  /// <remarks>
  ///   There fewer threads need to access the collection, the faster an implementation
  ///   can be. This is used as a template parameter to decide implementation.
  /// </remarks>
  enum class ConcurrentAccessBehavior {

    /// <summary>
    ///   Only one thread is taking data and another, but only one, is producing it
    /// </summary>
    SingleProducerSingleConsumer,

    /// <summary>
    ///   Only one thread is taking data, but multiple threads are adding data
    /// </summary>
    MultipleProducersSingleConsumer,

    /// <summary>
    ///   Any number of threads is taking data and any number of threads is adding it
    /// </summary>
    MultipleProducersMultipleConsumers

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Collection that can safely be used from multiple threads</summary>
  /// <typeparam name="TElement">Type of elements stored in the collection</typeparam>
  /// <typeparam name="accessBehavior">How the collection may be accesed by threads</typeparam>
  template<
    typename TElement,
    ConcurrentAccessBehavior accessBehavior = (
      ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
    )
  >
  class ConcurrentCollection {

    /// <summary>Destroys the concurrent collection</summary>
    public: virtual ~ConcurrentCollection() = default;

    /// <summary>Tries to append an element to the collection in a thread-safe manner</summary>
    /// <param name="element">Element that will be appended to the collection</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: virtual bool TryAppend(const TElement &element) = 0;

#if 0 // if the outcome is uncertain, move semantics mean the object is necessarily toast.
    /// <summary>
    ///   Tries to move-append an element to the collection in a thread-safe manner
    /// </summary>
    /// <param name="element">Element that will be move-appended to the collection</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: virtual bool TryAppend(TElement &&element) = 0;
#endif

    /// <summary>Tries to take an element from the collection</summary>
    /// <param name="element">Will receive the element taken from the collection</param>
    /// <returns>
    ///   True if an element was taken from the collection, false if the collection was empty
    /// </returns>
    public: virtual bool TryTake(TElement &element) = 0;

    /// <summary>Counts the numebr of elements current in the collection</summary>
    /// <returns>
    ///   The approximate number of elements that had been in the collection during the call
    /// </returns>
    public: virtual std::size_t Count() const = 0;

    /// <summary>Checks if the collection is empty</summary>
    /// <returns>True if the collection had been empty during the call</returns>
    public: virtual bool IsEmpty() const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTCOLLECTION_H
