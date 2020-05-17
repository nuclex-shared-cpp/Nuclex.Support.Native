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

#ifndef NUCLEX_SUPPORT_TEXT_FEEDBACKRECEIVER_H
#define NUCLEX_SUPPORT_TEXT_FEEDBACKRECEIVER_H

#include "Nuclex/Support/Config.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  // Split into separate interfaces for SetProgress() / SetStatus() / Rest ?
  //
  // Pro: no pointless methods for micro tasks
  // Con: multiple optional interfaces to check and implement
  //

  /// <summary>Interface that accepts feedback from a long-running task</summary>
  class FeedbackReceiver {

    /// <summary>Frees all resources owned by the feedback receiver</summary>
    public: NUCLEX_SUPPORT_API virtual ~FeedbackReceiver() = default;

    /// <summary>Updates the current progress of the operation</summary>
    /// <param name="progress">Achieved rogress in a range of 0.0 .. 1.0</param>
    /// <remarks>
    ///   Progress should stay within the specified range. Ideally, progress should never
    ///   go backwards, but that may be better than just freezing progress if your operation
    ///   encounters a major unexpected roadblock.
    /// </remarks>
    public: virtual void SetProgress(float progress) = 0;

    /// <summary>Updates the major operation status</summary>
    /// <param name="status">
    /// <remarks>
    ///   This is typically the text you'd want displayed in an applications status bar
    ///   or in a progress window. It shouldn't be too technical or change at a fast pace.
    /// </remarks>
    public: virtual void SetStatus(const std::string &status) = 0;

    /// <summary>Whether the feedback receiver is doing anything with the log messages</summary>
    /// <returns>True if the log messages are processed in any way, false otherwise</returns>
    /// <remarks>
    ///   Forming the log message strings may be non-trivial and cause memory allocations, too,
    ///   so by checking this method just once, you can skip all logging if they would be
    ///   discarded anyway.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual bool IsLogging() const { return false; }

    /// <summary>Logs a diagnostic message</summary>
    /// <param name="message">Message the operation wishes to log</param>
    /// <remarks>
    ///   Use this for diagnostic output that may help with debugging or verifying that
    ///   things are indeed happening the way you intended to. These messages typically
    ///   go into some log, a details window or are discarded outright.
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void LogMessage(const std::string &message) {
      (void)message;
    }

    /// <summary>Logs a warning</summary>
    /// <param name="warning">Warning the operation wishes to log</param>
    /// <remarks>
    ///   <para>
    ///     Use this if your operation encounters a problem that isn't fatal but means
    ///     that the outcome will not be as intended. Also use if your operation discovers
    ///     something that isn't the way it should be (i.e. a filename doesn't follow
    ///     conventions, data uses deprecated format, etc.)
    ///   </para>
    ///   <para>
    ///     Logged warnings may be displayed to the user, for example as a summary after
    ///     the operation completed with warnings.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void LogWarning(const std::string &warning) {
      (void)warning;
    }

    /// <summary>Logs an error</summary>
    /// <param name="error">Error the operation wishes to log</param>
    /// <remarks>
    ///   <para>
    ///     Only use this if the operation is bound to fail. An exception should be thrown
    ///     from the operation as a result.
    ///   </para>
    ///   <para>
    ///     The error logger may provide additional information beyond the exception
    ///     message and may be displayed to the user, for example in an error dialog after
    ///     the operation has failed.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API virtual void LogError(const std::string &error) {
      (void)error;
    }

#if defined(WOULD_BE_NICE_IF_PORTABLE)

    /// <summary>Whether the feedback receiver is checking logged messages at all</summary>
    /// <returns>True if the feedback receiver is checking logged messages, false otherwise</returns>
    /// <remarks>
    ///   If your messages are costly to form, you can check this property once in your
    ///   operation to see whether issuing log messages is needed at all.
    /// </remarks>
    public: inline bool IsLogging() const {
      //FeedbackReceiver *dummy = nullptr;
      //(FeedbackReceiver::void (*baseMethod)(const std::string &)) =
      return (
        (&this->LogMessage != &FeedbackReceiver::LogMessage) ||
        (&this->LogWarning != &FeedbackReceiver::LogWarning) ||
        (&this->LogError != &FeedbackReceiver::LogError)
      );
    }

#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_FEEDBACKRECEIVER_H
