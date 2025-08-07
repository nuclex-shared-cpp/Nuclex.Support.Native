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

#ifndef NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H
#define NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::u8string
#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t
#include <filesystem> // for std::filesystem::path

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a temporary file that is deleted when the scope is destroyed</summary>
  /// <remarks>
  ///   <para>
  ///     This is very useful for unit tests or if you're dealing with a poorly designed
  ///     library that can only read resources from the file system rather than providing
  ///     an abstract IO interface.
  ///   </para>
  ///   <para>
  ///     Usage is straight-forward:
  ///   <para>
  ///   <example>
  ///     <code>
  ///       void test() {
  ///         TemporaryFileScope tempFile(u8"xyz"); // file with custom prefix
  ///
  ///         // Write something into the file. Overloads are also provided for
  ///         // a buffer + length pair as well as for std::vector<std::byte>.
  ///         tempFile.SetFileContents(u8"Hello World!");
  ///
  ///         // ...do something that requires an actual file....
  ///         ::poorly_designed_library_load_message(tempFile.GetPath());
  ///
  ///         // The file is deleted again upon destruction of the temporary file scope
  ///       }
  ///     </code>
  ///   </example>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE TemporaryFileScope {

    /// <summary>Reserves and creates a unique temporary file</summary>
    /// <param name="namePrefix">Prefix for the temporary filename</param>
    public: NUCLEX_SUPPORT_API TemporaryFileScope(const std::u8string &namePrefix = u8"tmp");

    /// <summary>Deletes the temporary file again</summary>
    public: NUCLEX_SUPPORT_API ~TemporaryFileScope();

    /// <summary>Returns the full, absolute path to the temporary file</summary>
    /// <returns>The absolute path of the temporary file as an UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API const std::filesystem::path &GetPath() const {
      return this->path;
    }

    /// <summary>Reads the current contents of the file as a string</summary>
    /// <returns>The current contents of the file as a string</returns>
    public: NUCLEX_SUPPORT_API std::u8string GetFileContentsAsString() const;

    /// <summary>Reads the current contents of the file as a vector</summary>
    /// <returns>The current contents of the file as a vector</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::byte> GetFileContentsAsVector() const;

    /// <summary>Replaces the file contents with the specified string</summary>
    /// <param name="text">String whose contents will be written into the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(const std::u8string &text) {
      SetFileContents(reinterpret_cast<const std::byte *>(text.c_str()), text.length());
    }

    /// <summary>Replaces the file contents with the data in the specified vector</summary>
    /// <param name="contents">Contents that will be written into the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(const std::vector<std::byte> &contents) {
      SetFileContents(contents.data(), contents.size());
    }

    /// <summary>Replaces the file contents with the specified memory block</summary>
    /// <param name="contents">Memory block containing the new file contents</param>
    /// <param name="byteCount">Number of bytes that will be written to the file</param>
    public: NUCLEX_SUPPORT_API void SetFileContents(
      const std::byte *contents, std::size_t byteCount
    );

    /// <summary>The full path to the temporary file</summary>
    private: std::filesystem::path path;
    /// <summary>Memory used to store the open file handle</summary>
    private: std::uint8_t privateImplementationData[sizeof(std::uintptr_t)];

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_TEMPORARYFILESCOPE_H
