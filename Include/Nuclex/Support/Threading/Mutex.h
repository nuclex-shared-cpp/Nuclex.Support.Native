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

#ifndef NUCLEX_SUPPORT_THREADING_MUTEX_H
#define NUCLEX_SUPPORT_THREADING_MUTEX_H

#include "../Config.h"

#include <functional>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets only a single thread access a section of code at a time</summary>
  class Mutex {

    /// <summary>Initializes a new mutex</summary>
    public: NUCLEX_SUPPORT_API Mutex();

    /// <summary>Destroys the mutex</summary>
    public: NUCLEX_SUPPORT_API ~Mutex();

    /// <summary>Enters the mutex</summary>
    public: NUCLEX_SUPPORT_API void Lock();

    /// <summary>Tries to enter the mutex</summary>
    /// <returns>True if the mutex was entered, false if it was occupied</returns>
    public: NUCLEX_SUPPORT_API bool TryLock();

    /// <summary>Exits the mutex</summary>
    public: NUCLEX_SUPPORT_API void Unlock();

    private: Mutex(const Mutex &);
    private: Mutex &operator =(const Mutex &);

    /// <summary>Stores private implementation details</summary>
    private: struct Implementation;
    /// <summary>Private implementation details</summary>
    private: Implementation *implementation;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_MUTEX_H
