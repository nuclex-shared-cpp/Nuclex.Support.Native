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

#ifndef NUCLEX_SUPPORT_THREADING_PROCESS_H
#define NUCLEX_SUPPORT_THREADING_PROCESS_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Events/Event.h" // TODO: need concurrent event

#include <chrono> // for std::chrono::milliseconds
#include <vector> // for std::vector
#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps an external executable running as an independent process</summary>
  class Process {

    /// <summary>Event that is fired whenever the process writes to stdout</summary>
    public: Nuclex::Support::Events::Event<void(const std::string &)> StdOutWritten;
    /// <summary>Event that is fired whenever the process writes to stderr</summary>
    public: Nuclex::Support::Events::Event<void(const std::string &)> StdErrWritten;

    /// <summary>Initializes a new process without starting it</summary>
    public: Process(const std::string &executablePath);
    /// <summary>Kills the external process and waits until it is gone</summary>
    public: ~Process();

    /// <summary>
    ///   Starts the external process, passing the specified command-line arguments along
    /// </summary>
    /// <param name="arguments">Arguments that will be passed to the external process</param>
    /// <param name="prependExecutableName">
    ///   The first argument passed is normally the name of the executable itself. Leaving this
    ///   set to 'true' will automatically prepend the executable name to the argument list.
    /// </param>
    public: void Start(
      const std::vector<std::string> &arguments = std::vector<std::string>(),
      bool prependExecutableName = true
    );

    /// <summary>Checks whether the process is still running</summary>
    /// <returns>True if the process was still running, false otherwise</returns>
    public: bool IsRunning() const;

    /// <summary>Waits for the process to exit normally</summary>
    /// <param name="patience">
    ///   Time the method will wait for the process to exit.
    /// </param>
    /// <returns>
    ///   True if the process exited within the allotted time, false if it is still running.
    /// </returns>
    public: bool Wait(
      std::chrono::milliseconds patience = std::chrono::milliseconds(30000)
    ) const;

    /// <summary>Waits for the process to exit normally and returns its exit code</summary>
    /// <param name="patience">
    ///   Time the method will wait for the process to exit. If the process does no exit within
    ///   this time, an exception will be thrown.
    /// </param>
    /// <returns>
    ///   The exit code returned by the process (most programs adhere to returning 0 if
    ///   everything went well and 1 if there was a problem).
    /// </returns>
    public: int Join(std::chrono::milliseconds patience = std::chrono::milliseconds(30000));

    /// <summary>Attempts to terminate the external process</summary>
    /// <param name="patience">
    ///   Time given to the process to respond to a request to gracefully terminate.
    ///   If zero, the process is forcefully killed immediately.
    /// </param>
    /// <remarks>
    ///   This will first attempt to gracefully exit the running process (by sending it
    ///   a SIGTERM signal or closing its main window). If this doesn't result in the process
    ///   terminating within the a grace period, this method will attempt to terminate
    ///   the process forcefully via either SIGKILL or by aborting its process.
    /// </remarks>
    public: void Kill(std::chrono::milliseconds patience = std::chrono::milliseconds(5000)) {}

    //public: std::any GetProcessId() const {}

    /// <summary>Path to the executable as which this process is running</summary>
    private: std::string executablePath;

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: const PlatformDependentImplementationData &getImplementationData() const;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
    private: union {
      /// <summary>Platform dependent process and file handles used for the process</summary>
      PlatformDependentImplementationData *implementationData;
      /// <summary>Used to hold the platform dependent implementation data if it fits</summary>
      /// <remarks>
      ///   Small performance / memory fragmentation improvement.
      ///   This avoids a micro-allocation for the implenmentation data structure in most cases.
      /// </remarks>
#if defined(NUCLEX_SUPPORT_WIN32)
      unsigned char implementationDataBuffer[32];
#else // Posix and Linux
      unsigned char implementationDataBuffer[24];
#endif
    };

  };


  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
