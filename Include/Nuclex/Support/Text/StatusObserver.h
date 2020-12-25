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

#ifndef NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H
#define NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H

#include "Nuclex/Support/Config.h"

#include <string>

// DONE: FeedbackReceiver is pretty UI-centric (either console or GUI) - rename?
//   UiFeedbackReceiver?
//   StatusObserver?

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interface that accepts feedback from a long-running task</summary>
  class StatusObserver {

    /// <summary>Frees all resources owned by the feedback receiver</summary>
    public: NUCLEX_SUPPORT_API virtual ~StatusObserver() = default;

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
    ///   Console applications can print the string reported through this method, so also
    ///   avoid calling it repeatedly if the text hasn't changed.
    /// </remarks>
    public: virtual void SetStatus(const std::string &status) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STATUSOBSERVER_H
