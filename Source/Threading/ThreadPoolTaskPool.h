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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

#include "ThreadPoolConfig.h"

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages reusable tasks for the thread pool</summary>
  /// <typeparam name="TSubmittedTask">Store all informations about a submitted task</typeparam>
  /// <typeparam name="PayloadOffset">Offset at which the variable payload begins</typeparam>
  template<typename TSubmittedTask, std::size_t PayloadOffset>
  class ThreadPoolTaskPool {

    public: ThreadPoolTaskPool() {
    }

    public: TSubmittedTask *GetNewTask(std::size_t payloadSize) {
      std::size_t totalRequiredMemory = (PayloadOffset + payloadSize);
      if(totalRequiredMemory >= ThreadPoolConfig::SubmittedTaskReuseLimit) {
        std::unique_ptr<std::uint8_t[]> taskMemory(
          new std::uint8_t[totalRequiredMemory]
        );

      }


    }

    public: void ReturnTask(TSubmittedTask *submittedTask) {

    }

    /// <summary>Tasks that have been given back and wait for their reuse</summary>
    private: moodycamel::ConcurrentQueue<TSubmittedTask *> returnedTasks;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H
