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

#ifndef NUCLEX_SUPPORT_THREADING_STOPTOKEN_H
#define NUCLEX_SUPPORT_THREADING_STOPTOKEN_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Threading/Gate.h"
#include "Nuclex/Support/Errors/CanceledError.h"

#include <memory> // for std::enable_shared_from_this, std::shared_ptr
#include <atomic> // for std::atomic

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Passed to background processes to tell when when they should cancel</summary>
  class NUCLEX_SUPPORT_TYPE StopToken {

    /// <summary>Initializes a new stop token</summary>
    protected: NUCLEX_SUPPORT_API StopToken() :
      Canceled(false), CancellationGate(false), CancellationReason() {}

    /// <summary>Frees all resources owned by the stop token</summary>
    public: NUCLEX_SUPPORT_API virtual ~StopToken() = default;

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Checks whether a cancellation has occured</summary>
    public: NUCLEX_SUPPORT_API inline bool IsCanceled() const;

    /// <summary>Throws an exception if a cancellation has occured</summary>
    public: NUCLEX_SUPPORT_API inline void ThrowIfCanceled() const;

    // ----------------------------------------------------------------------------------------- //

    /// <summary>
    ///   Waits for the token to be canceled, returns immediately if it already is canceled
    /// </summary>
    public: NUCLEX_SUPPORT_API inline void Wait() const;

    /// <summary>
    ///   Waits for the token to be canceled, returns immediately if it already is canceled.
    /// </summary>
    /// <param name="patience">How long to wait for the token to be canceled</param>
    /// <returns>
    ///   True if the token was canceled, false if the patience time has elapsed
    /// </returns>
    /// <remarks>
    ///   You can use this method if you implement a repeating background task, for example.
    ///   It will soundly sleep without consuming CPU cycles and wake up either when its
    ///   wait time has expired or immediately if it is cancelled. For more complex scenarios,
    ///   such as a background task that needs to be woken up when its interval changes,
    ///   more complex solutions (such as POSIX condition variables) are needed.
    /// </remarks>
    public: NUCLEX_SUPPORT_API inline bool WaitFor(
      const std::chrono::microseconds &patience
    ) const;

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Cancellation reason, doubles are cancellation flag if set</summary>
    protected: std::atomic<bool> Canceled;
    /// <summary>Gate that will be opened when the token is cancelled</summary>
    protected: Gate CancellationGate;
    /// <summary>Why cancellation happened, optionally provided by the canceling side</summary>
    protected: std::string CancellationReason;

  };

  // ------------------------------------------------------------------------------------------- //

  inline bool StopToken::IsCanceled() const {
    return this->Canceled.load(std::memory_order::memory_order_relaxed);
  }

  // ------------------------------------------------------------------------------------------- //

  inline void StopToken::ThrowIfCanceled() const {
    if(IsCanceled()) {
      std::atomic_thread_fence(std::memory_order::memory_order_acquire);
      throw Nuclex::Support::Errors::CanceledError(this->CancellationReason);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline void StopToken::Wait() const {
    this->CancellationGate.Wait();
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool StopToken::WaitFor(const std::chrono::microseconds &patience) const {
    return this->CancellationGate.WaitFor(patience);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_STOPTOKEN_H
