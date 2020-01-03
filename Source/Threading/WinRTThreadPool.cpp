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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/WinRTThreadPool.h"

#if defined(NUCLEX_SUPPORT_WINRT)

#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

using namespace Windows::Foundation;
using namespace Windows::System::Threading;

namespace {

  // ------------------------------------------------------------------------------------------- //

  void threadPoolWorkCallback(IAsyncAction ^) {
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  WinRTThreadPool::WinRTThreadPool() {}

  // ------------------------------------------------------------------------------------------- //

  WinRTThreadPool::~WinRTThreadPool() {}

  // ------------------------------------------------------------------------------------------- //

  std::size_t WinRTThreadPool::CountMaximumParallelTasks() const {
    static SYSTEM_INFO systemInfo = SYSTEM_INFO();

    if(systemInfo.dwNumberOfProcessors == 0) {
      ::GetNativeSystemInfo(&systemInfo);
    }

    return static_cast<std::size_t>(systemInfo.dwNumberOfProcessors);
  }

  // ------------------------------------------------------------------------------------------- //

  void WinRTThreadPool::AddTask(
    const std::function<void()> &task, std::size_t count /* = 1 */
  ) {
    WorkItemHandler ^workItemHandler = ref new WorkItemHandler(
      [task](IAsyncAction ^) throw() { // Capture 'task' by value, so it remains valid
        try {
          task();
        }
        catch(const std::exception &) {
          std::unexpected();
        }
      }
    );
    while(count > 0) {
      Windows::System::Threading::ThreadPool::RunAsync(workItemHandler);
      --count;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WINRT)
