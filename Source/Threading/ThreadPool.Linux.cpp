#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/ThreadPool.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include <cassert> // for assert()

//#include <stdexcept>
//#include <algorithm>

//#include <unistd.h> // for ::sysconf()
//#include <limits.h> // 
#include <sys/sysinfo.h> // for ::get_nprocs()

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the process</summary>
    public: PlatformDependentImplementationData() :
      ProcessorCount(static_cast<std::size_t>(::get_nprocs())) {}

    /// <summary>Number of logical processors in the system</summary>
    public: std::size_t ProcessorCount; 

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool() :
    implementationData(nullptr) {

    // If this assert hits, the buffer size assumed by the header was too small.
    // Things will still work, but we have to resort to an extra allocation.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::ThreadPool fits in buffer"
    );

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      new(this->implementationDataBuffer) PlatformDependentImplementationData();
    } else {
      this->implementationData = new PlatformDependentImplementationData();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    PlatformDependentImplementationData &impl = getImplementationData();
    // TODO: Place shutdown code here

    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(!implementationDataFitsInBuffer) {
      delete this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::CountMaximumParallelTasks() const {
    const PlatformDependentImplementationData &impl = getImplementationData();
    return impl.ProcessorCount;
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  void ThreadPool::AddTask(
    const std::function<void()> &task, std::size_t count /* = 1 */
  ) {
    (void)task;
    (void)count;
    // TODO: Implement thread pool on Linux
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const ThreadPool::PlatformDependentImplementationData &ThreadPool::getImplementationData(
  ) const {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
      );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<const PlatformDependentImplementationData *>(
        this->implementationDataBuffer
        );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementationData &ThreadPool::getImplementationData() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
      );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<PlatformDependentImplementationData *>(
        this->implementationDataBuffer
        );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX)
