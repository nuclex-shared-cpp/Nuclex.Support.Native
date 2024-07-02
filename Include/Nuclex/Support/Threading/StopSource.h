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

// This is sort of a nice idea, but also a bit cumbersome to use
//
// .NET task use this concept, but the only advantage this gives is that lambdas and
// any random junk can be executed as a task ad-hoc without having to inherit from
// a task base class to support cancellation.
//
// I think requiring classes to be types gives more advantages than externalizing
// the concept of cancellation. For ad-hoc junk, you can still make a task that ignores
// cancellation (or passes it to the ad-hoc junk) and wrap that.
//
#ifndef NUCLEX_SUPPORT_THREADING_STOPSOURCE_H
#define NUCLEX_SUPPORT_THREADING_STOPSOURCE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Threading/StopToken.h"

#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allows cancelling all tasks holding the source's stop token</summary>
  /// <remarks>
  ///   <para>
  ///     This is very similar to <code>std::stop_source</code> introduced with C++20,
  ///     or Microsoft's concept of &quot;cancellation tokens&quot; found in their PPL,
  ///     C++ REST SDK and in .NET
  ///   </para>
  ///   <para>
  ///     Basically, the initial launcher of a background task provides the task with
  ///     a stop token (as a parameter to the initiating method). The task is then
  ///     supposed to hold onto the stop token and stop running when the stop token's
  ///     <see cref="StopToken.IsCanceled" /> property is set to true
  ///     (by sporadically checking it at opportune times for interruption).
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE StopSource : protected StopToken {

    /// <summary>
    ///   Builds a new stop source, required to prevent stack allocations
    /// </suimmary>
    /// <returns>The new stop source</returns>
    public: NUCLEX_SUPPORT_API static std::shared_ptr<StopSource> Create();

    /// <summary>Initializes a new stop source</summary>
    protected: NUCLEX_SUPPORT_API StopSource() = default;

    /// <summary>Frees all resources used by the stop source</summary>
    public: NUCLEX_SUPPORT_API virtual ~StopSource() override = default;

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Returns the source's stop token</summary>
    /// <returns>The stop token responding to the source</returns>
    public: NUCLEX_SUPPORT_API std::shared_ptr<const StopToken> GetToken() const {
      return this->watcher.lock();
    }

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Triggers the cancellation, signaling the stop token</summary>
    /// <param name="reason">
    ///   Optional reason for the cancellation, included in exception message when
    ///   <see cref="StopToken.ThrowIfCanceled" /> is used.
    /// </param>
    public: NUCLEX_SUPPORT_API void Cancel(const std::string &reason = std::string()) {
      assert((IsCanceled() == false) && u8"Cancellation is triggered only once");

      this->CancellationReason = reason;
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      this->Canceled.store(true, std::memory_order::memory_order_relaxed);
    }

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Watcher handed out by the <see cref="GetToken" /> method</summary>
    /// <remarks>
    ///   This is actually a weak_ptr to this instance itself under its base class.
    ///   It cannot be a shared_ptr since then, the instance would self-reference,
    ///   preventing the reference counter from ever reaching zero, causing a memory leak.
    /// </remarks>
    private: std::weak_ptr<StopToken> watcher;

  };

  // ------------------------------------------------------------------------------------------- //

  inline std::shared_ptr<StopSource> StopSource::Create() {
    struct ConstructibleStopSource : public StopSource {
      ConstructibleStopSource() = default;
      virtual ~ConstructibleStopSource() override = default;
    };
    std::shared_ptr<StopSource> result = std::make_shared<ConstructibleStopSource>();

    #if 0
    // This seems to be a spot where compilers may report an error.
    // The reason being that, unlike the StopSource::Create() method,
    // std::static_pointer_cast() does not have access to protected members of
    // this class, and that includes the protected base class, unfortunately.
    result->watcher = (
      std::static_pointer_cast<StopToken, StopSource>(result)
    );
    #else
    // This is pure shared_ptr villainy, but for single-inheritance,
    // the pointer to base and derived will be at the same address
    // on all supported compilers, so we can do this and avoid having
    // to declare the base class as public (which would suck!)
    //
    // Make a weak_ptr to a stop token that's actually us in disguise
    result->watcher = *reinterpret_cast<std::shared_ptr<StopToken> *>(&result);
    #endif

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_STOPSOURCE_H
