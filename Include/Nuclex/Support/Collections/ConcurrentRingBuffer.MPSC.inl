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

#if !defined(NUCLEX_SUPPORT_COLLECTIONS_CONCURRENTRINGBUFFER_H)
#error This file must be included through via ConcurrentRingBuffer.h
#endif

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Fixed-size list that can safely be used from multiple threads</summary>
  ///   <para>
  ///     <strong>Thread safety:</strong> any thread depending on selected specialization
  ///   </para>
  ///   <para>
  ///     <strong>Container type:</strong> bounded ring buffer
  ///   </para>
  template<typename TElement>
  class ConcurrentRingBuffer<TElement, ConcurrentAccessBehavior::MultipleProducersSingleConsumer> {

    #pragma region struct ItemMemoryDeleter

    /// <summary>Frees the buffer's item memory when an exception happens</summary>
    /// <remarks>
    ///   Could use my scope guard implementation here, but this reduces dependencies
    /// </remarks>
    private: struct ItemMemoryDeleter {

      /// <summary>Initializes a new buffer item memory deleter</summary>
      /// <param name="itemMemory">Memory block that will be deleted on destruction</param>
      public: ItemMemoryDeleter(std::uint8_t *itemMemory) :
        itemMemory(itemMemory),
        armed(true) {}
      
      /// <summary>Deletes the item memory buffer</summary>
      public: ~ItemMemoryDeleter() {
        if(this->armed) {
          delete[] this->itemMemory;
        }
      }

      /// <summary>Disarms the deleter to no longer delete the buffer on destruction</summary>
      public: void Disarm() {
        this->armed = false;
      }

      /// <summary>Memory buffer that will be deleted on destruction</summary>
      private: std::uint8_t *itemMemory;
      /// <summary>Whether the deleter instance will delete the memory buffer</summary>
      private: bool armed;

    };

    #pragma endregion // struct ItemMemoryDeleter

    /// <summary>Initializes a new concurrent queue for a single producer and consumer</summary>
    /// <param name="capacity">Maximum amount of items the queue can hold</param>
    public: ConcurrentRingBuffer(std::size_t capacity) :
      capacity(capacity + 1),
      itemMemory(nullptr),
      itemStatus(nullptr) {
      std::uint8_t *buffer = new std::uint8_t[sizeof(TElement[2]) * (capacity + 1U) / 2U];
      {
        //auto itemMemoryDeleter = ON_SCOPE_EXIT_TRANSACTION {
        //  delete[] buffer;
        //};

        ItemMemoryDeleter itemMemoryScope(buffer);
        this->itemStatus = new std::atomic<std::uint8_t>[capacity + 1U];
        itemMemoryScope.Disarm();

        //itemMemoryDeleter.Commit();
      }
      this->itemMemory = reinterpret_cast<TElement *>(buffer);
    }
    
    /// <summary>Frees all memory owned by the concurrent queue and the items therein</summary>
    public: ~ConcurrentRingBuffer() {
      if constexpr(!std::is_trivially_destructible<TElement>::value) {
/*      
        std::size_t safeReadIndex = this->readIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        int safeWriteIndex = this->writeIndex.load(
          std::memory_order::memory_order_consume // consume: while() below carries dependency
        );
        while(safeReadIndex != positiveModulo(safeWriteIndex)) {
          reinterpret_cast<TElement *>(this->itemMemory)[safeReadIndex].~TElement();
          safeReadIndex = (safeReadIndex + 1) % this->capacity;
        }
*/
      }

      delete[] this->itemStatus;
#if !defined(NDEBUG)
      this->itemStatus = nullptr;
#endif
      delete[] reinterpret_cast<std::uint8_t *>(this->itemMemory);
#if !defined(NDEBUG)
      this->itemMemory = nullptr;
#endif
    }

    /// <summary>Returns the maximum number of items the queue can hold</summary>
    /// <returns>The maximum number of items the queue can hold</returns>
    public: std::size_t GetCapacity() const { return this->capacity - 1U; }

    /// <summary>Number of items the ring buffer can hold</summary>
    private: const std::size_t capacity;
    /// <summary>Memory block that holds the items currently stored in the queue</summary>
    private: TElement *itemMemory;
    /// <summary>Status of items in buffer, 0: empty, 1: filling, 2: present, 3: gap</summary>
    private: std::atomic<std::uint8_t> *itemStatus;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
