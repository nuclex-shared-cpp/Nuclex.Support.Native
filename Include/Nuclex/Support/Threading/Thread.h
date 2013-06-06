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

#ifndef NUCLEX_SUPPORT_THREADING_THREAD_H
#define NUCLEX_SUPPORT_THREADING_THREAD_H

#include <cstddef>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides supporting methods for threads</summary>
  class Thread {

    /// <summary>Lets the calling thread wait for the specified amount of time</summary>
    /// <param name="microseconds">Duration for which the thread will wait</param>
    public: static void Sleep(std::size_t microseconds);

    /// <summary>Determines whether the calling thread belongs to the thread pool</summary>
    /// <returns>True if the calling thread belongs to the thread pool</returns>
    public: static bool BelongsToThreadPool();

    private: Thread(const Thread &);
    private: Thread&operator =(const Thread &);

  };


  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
