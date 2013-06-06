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

#ifndef NUCLEX_SUPPORT_THREADING_READERWRITERLOCK_H
#define NUCLEX_SUPPORT_THREADING_READERWRITERLOCK_H

#include "../Config.h"

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets only a single thread access a section of code at a time</summary>
  class ReaderWriterLock {

    /// <summary>Initializes a new mutex</summary>
    public: NUCLEX_SUPPORT_API ReaderWriterLock();

    /// <summary>Destroys the mutex</summary>
    public: NUCLEX_SUPPORT_API ~ReaderWriterLock();

    /// <summary>Enters the lock as a reader</summary>
    public: NUCLEX_SUPPORT_API void LockAsReader() {
      std::size_t count = this->enteredReaderCount.fetch_add(1);
      if(count >= halfMax) {
        //this->writerMutex.lock();
      }
    }

    /// <summary>Enters the lock as a writer</summary>
    public: NUCLEX_SUPPORT_API void LockAsWriter() {
      //std::size_t count = this->enteredReaderCount.load(std::memory_order_acquire);

    }

    /// <summary>Tries to enter the lock as a reader</summary>
    /// <returns>True if the lock was entered, false if it was occupied</returns>
    public: NUCLEX_SUPPORT_API bool TryLockAsReader() {
      return false;
    }

    /// <summary>Tries to enter the lock as a writer</summary>
    /// <returns>True if the lock was entered, false if it was occupied</returns>
    public: NUCLEX_SUPPORT_API bool TryLockAsWriter() {
      return false;
    }

    /// <summary>Exits the lock as a reader</summary>
    public: NUCLEX_SUPPORT_API void UnlockAsReader() {
      std::size_t count = this->enteredReaderCount.fetch_sub(1);
      if(count == halfMax + 1) {
        // writer is waiting and last reader has left, set "writer can continue" event
      }
    }

    /// <summary>Exits the lock as a writer</summary>
    public: NUCLEX_SUPPORT_API void UnlockAsWriter() {
    }

    private: ReaderWriterLock(const ReaderWriterLock &);
    private: ReaderWriterLock &operator =(const ReaderWriterLock &);

    private: static const std::size_t halfMax = static_cast<std::size_t>(-1) / 2;

    /// <summary>Number of readers that have currently entered the lock</summary>
    private: std::atomic_size_t enteredReaderCount;

    //private: std::mutex writerMutex;
    //private: std::condition_variable

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_READERWRITERLOCK_H
