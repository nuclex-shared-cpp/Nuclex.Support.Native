#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
#define NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H

#include "Nuclex/Support/Config.h"

#include <atomic>

#if 0

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  template<typename TElement>
  class ConcurrentSet {

    #pragma region struct Node

    /// <summary>Stores a list entry</summary>
    private: struct Node {
      /// <summary>Initializes a new list node</summary>
      /// <param name="element">Element the node will carry</param>
      public: Node(const TElement &element) :
        Next(nullptr),
        ReferenceCount(1),
        Element(element) {}

      /// <summary>Points to the next element in the list</summary>
      public: std::atomic<Node *> Next;
      // <summary>Number of references that exist to this node</summary>
      //public: std::atomic_size_t ReferenceCount;
      /// <summary>Stores the element itself</summary>
      public: TElement Element;
    };

    #pragma endregion // struct Node

    /// <summary>Initializes a new set</summary>
    public: ConcurrentSet() :
      head(nullptr) {}

    /// <summary>Adds an element to the set</summary>
    /// <param name="element">Element that will be added to the set</param>
    public: void Add(const TElement &element) {
      Node *newNode = new Node(element);

      // Acquire the current list head so it can be replaced
      Node *currentHead = this->head.load(std::memory_order_acquire);

      bool wasExchanged;
      do {
        // If we successfully replace the list head (and nobody snatched it away),
        // the next pointer needs to point to the old head node.
        newNode->Next.store(std::memory_order_release);

        // Only replace the head if it was still what we prepared the new node for.
        // If the head was replaced by something else in the meantime, it will be
        // stored in currentHead, so we can try again in the next loop.
        wasExchanged = this->head.compare_exchange_strong(
          currentHead, nodeToInsert, std::memory_order_acq_rel
        );
      } while(!wasExchanged);

      ++count;
    }

    /// <summary>Removes the specified 
    public: bool Remove(const TElement &element) {
      this->threadCount.fetch_add(1, std::memory_order_acquire);

      Node *currentHead = this->head.load(std::memory_order_acquire);
      if(currentHead->Element == element) {
        for(;;) {
          //this->head.fetch_add();
        }
      }
      // Uh-oh. I don't have the slightest clue how I could safely delete the node
      // 
      // 1. Increment thread count
      // 2. Find element to remove
      // 3. Replace previous node's next pointer (CAS loop)
      // 4. Add removed element to garbage list
      // 5. Decrement thread count
      // 6. If thread count was 0, guarantee that no thread was accessing any node
      //    contained in the garbage list at the time of the call
      // 6a. Replace garbage head with garbage next (CAS)
      // 6b. If CAS failed, return (another thread is adding to the garbage)
      //     If CAS succeeded, delete extracted garbage node, go to 6a.
    }

    /// <summary>First node in the linked list</summary>
    private: std::atomic<Node *> head;
    /// <summary>Garbage list, to be freed when the last thread leaves</summary>
    private: std::atomic<Node *> garbage;
    /// <summary>Stores the approximate number of elements in the list</summary>
    private: std::atomic_size_t count;
    /// <summary>Number of threads currently using the set</summary>
    private: std::atomic_size_t threadCount;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // 0

#endif // NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTSET_H
