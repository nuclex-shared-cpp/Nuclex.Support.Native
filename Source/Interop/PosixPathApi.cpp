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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "PosixPathApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <limits.h> // for PATH_MAX
#include <sys/stat.h> // for ::stat()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Interop {

  // ------------------------------------------------------------------------------------------- //

  bool PosixPathApi::IsPathRelative(const std::u8string &path) {
    std::u8string::size_type length = path.length();
    if(length == 0) {
      return true;
    }

    if(length >= 2) {
      if((path[0] == '~') && (path[1] == L'/')) {
        return false;
      }
    }

    return (path[0] != '/');
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixPathApi::AppendPath(std::u8string &path, const std::u8string &extra) {
    std::u8string::size_type length = path.length();
    if(length == 0) {
      path.assign(extra);
    } else {
      if(path[length - 1] != '/') {
        path.push_back('/');
      }
      path.append(extra);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixPathApi::RemoveFileFromPath(std::u8string &path) {
    std::u8string::size_type lastBackslashIndex = path.find_last_of('/');
    if(lastBackslashIndex != std::u8string::npos) {
      path.resize(lastBackslashIndex + 1); // Keep the slash on
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool PosixPathApi::DoesFileExist(const std::filesystem::path &path) {
    struct ::stat fileStatus;

    std::string pathChars = path.string();
    int result = ::stat(pathChars.c_str(), &fileStatus);
    if(result == -1) {
      int errorNumber = errno;

      // This is an okay outcome for us: the file or directory does not exist.
      if((errorNumber == ENOENT) || (errorNumber == ENOTDIR)) {
        return false;
      }

      std::u8string errorMessage(u8"Could not obtain file status for '");
      errorMessage.append(pathChars.begin(), pathChars.end());
      errorMessage.append(u8"'");

      Interop::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixPathApi::GetTemporaryDirectory(std::u8string &path) {
    const static char8_t *tmpDir = u8"TMPDIR";
    const static std::string tmpDirString(tmpDir, tmpDir + 6);
    const static char8_t *tmp = u8"TMP";
    const static std::string tmpString(tmp, tmp + 3);
    const static char8_t *temp = u8"TEMP";
    const static std::string tempString(temp, temp + 4);

    const char *tempDirectory = ::getenv(tmpDirString.c_str());
    if(tempDirectory == nullptr) {
      tempDirectory = ::getenv(tmpString.c_str());
      if(tempDirectory == nullptr) {
        tempDirectory = ::getenv(tempString.c_str());
        if(tempDirectory == nullptr) {
          // This is safe (part of the file system standard and Linux standard base),
          // but we wanted to honor any possible user preferences first.
          path = std::u8string(u8"/tmp", 4);
          return;
        }
      }
    }

    path.assign(std::move(Text::StringConverter::Utf8FromChar(tempDirectory)));
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Interop

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
