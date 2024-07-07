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

#include "Nuclex/Support/Settings/IniSettingsStore.h"
#include "Nuclex/Support/Text/LexicalCast.h"
#include "Nuclex/Support/ScopeGuard.h"

#include "IniDocumentModel.h"

#if defined(NUCLEX_SUPPORT_LINUX)
#include "../Platform/LinuxFileApi.h"
#elif defined(NUCLEX_SUPPORT_WINDOWS)
#include "../Platform/WindowsFileApi.h"
#else
#include "../Platform/PosixFileApi.h"
#endif

#include <memory> // for std::unique_ptr

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore() :
    privateImplementationData(nullptr),
    modified(false) {}

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore(const std::string &iniFilePath) :
    privateImplementationData(nullptr),
    modified(false) {
    Load(iniFilePath);
  }

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::IniSettingsStore(
    const std::uint8_t *iniFileContents, std::size_t iniFileByteCount
  ) :
    privateImplementationData(nullptr),
    modified(false) {
    Load(iniFileContents, iniFileByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  IniSettingsStore::~IniSettingsStore() {
    if(this->privateImplementationData != nullptr) {
      delete reinterpret_cast<IniDocumentModel *>(this->privateImplementationData);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Load(const std::string &iniFilePath) {
    std::vector<std::uint8_t> contents;

#if defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(iniFilePath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
          fileDescriptor, contents.data() + offset, 4096
        );
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset); // drop the 4k we added for appending
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          offset += readByteCount;
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    {
      ::HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForReading(iniFilePath);
      ON_SCOPE_EXIT { Platform::WindowsFileApi::CloseFile(fileHandle); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Platform::WindowsFileApi::Read(
          fileHandle, contents.data() + offset, 4096
        );
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          offset += readByteCount;
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#else
    {
      ::FILE *file = Platform::PosixFileApi::OpenFileForReading(iniFilePath);
      ON_SCOPE_EXIT { Platform::PosixFileApi::Close(file); };

      contents.resize(4096);
      for(std::size_t offset = 0;;) {
        std::size_t readByteCount = Platform::PosixFileApi::Read(
          file, contents.data() + offset, 4096
        );
        if(readByteCount == 0) { // 0 bytes are only returned at the end of the file
          contents.resize(offset);
          break;
        } else { // 1 or more bytes returned, increase buffer for another round
          offset += readByteCount;
          contents.resize(offset + 4096); // extend so that 4k bytes are available again
        }
      }
    }
#endif

    Load(contents.data(), contents.size());
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Load(const std::uint8_t *iniFileContents, std::size_t iniFileByteCount) {
    std::unique_ptr<IniDocumentModel> newDocumentModel(
      new IniDocumentModel(iniFileContents, iniFileByteCount)
    );
    if(this->privateImplementationData != nullptr) {
      delete reinterpret_cast<IniDocumentModel *>(this->privateImplementationData);
    }
    this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
      newDocumentModel.release()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::Save(const std::string &iniFilePath) const {
#if defined(NUCLEX_SUPPORT_LINUX)
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForWriting(iniFilePath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      std::size_t bytesWritten;
      if(this->privateImplementationData == nullptr) {
        bytesWritten = 0;
      } else {
        bytesWritten = reinterpret_cast<const IniDocumentModel *>(
          this->privateImplementationData
        )->Serialize(
          &fileDescriptor,
          [](void *context, const std::uint8_t *buffer, std::size_t byteCount) {
            Platform::LinuxFileApi::Write(
              *reinterpret_cast<int *>(context), buffer, byteCount
            );
          }
        );
      }

      Platform::LinuxFileApi::SetLength(fileDescriptor, bytesWritten);
      Platform::LinuxFileApi::Flush(fileDescriptor);
    }
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    {
      ::HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForWriting(iniFilePath);
      ON_SCOPE_EXIT { Platform::WindowsFileApi::CloseFile(fileHandle); };

      if(this->privateImplementationData == nullptr) {
        reinterpret_cast<const IniDocumentModel *>(this->privateImplementationData)->Serialize(
          &fileHandle,
          [](void *context, const std::uint8_t *buffer, std::size_t byteCount) {
            Platform::WindowsFileApi::Write(
              *reinterpret_cast<::HANDLE *>(context), buffer, byteCount
            );
          }
        );
      }

      Platform::WindowsFileApi::SetLengthToFileCursor(fileHandle);
      Platform::WindowsFileApi::FlushFileBuffers(fileHandle);
    }
#else
    {
      ::FILE *file = Platform::PosixFileApi::OpenFileForWriting(iniFilePath, true);
      ON_SCOPE_EXIT { Platform::PosixFileApi::Close(file); };

      std::size_t bytesWritten;
      if(this->privateImplementationData == nullptr) {
        bytesWritten = 0;
      } else {
        bytesWritten = reinterpret_cast<const IniDocumentModel *>(
          this->privateImplementationData
        )->Serialize(
          file,
          [](void *context, const std::uint8_t *buffer, std::size_t byteCount) {
            Platform::PosixFileApi::Write(
              reinterpret_cast<::FILE *>(context), buffer, byteCount
            );
          }
        );
      }

      //Platform::PosixFileApi::SetLength(file, bytesWritten);
      Platform::PosixFileApi::Flush(file);
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::uint8_t> IniSettingsStore::Save() const {
    if(this->privateImplementationData == nullptr) {
      return std::vector<std::uint8_t>();
    } else {
      return reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->Serialize();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::HasChangedSinceLoad() const {
    return this->modified;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniSettingsStore::GetAllCategories() const {
    return reinterpret_cast<const IniDocumentModel *>(
      this->privateImplementationData
    )->GetAllSections();
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniSettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    return reinterpret_cast<const IniDocumentModel *>(
      this->privateImplementationData
    )->GetAllProperties(categoryName);
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::DeleteCategory(const std::string &categoryName) {
    if(this->privateImplementationData == nullptr) {
      return false;
    } else {
      return reinterpret_cast<IniDocumentModel *>(
        this->privateImplementationData
      )->DeleteSection(categoryName);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniSettingsStore::DeleteProperty(
    const std::string &categoryName, const std::string &propertyName
  ) {
    if(this->privateImplementationData == nullptr) {
      return false;
    } else {
      return reinterpret_cast<IniDocumentModel *>(
        this->privateImplementationData
      )->DeleteProperty(categoryName, propertyName);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<bool> IniSettingsStore::RetrieveBooleanProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<bool>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        const std::string &stringValue = value.value();
        switch(stringValue.length()) {
          case 1: {
            return (stringValue[0] == '1');
          }
          case 2: {
            return (
              ((stringValue[0] == 'o') || (stringValue[0] == 'O')) &&
              ((stringValue[1] == 'n') || (stringValue[1] == 'N'))
            );
          }
          case 3: {
            return (
              ((stringValue[0] == 'y') || (stringValue[0] == 'Y')) &&
              ((stringValue[1] == 'e') || (stringValue[1] == 'E')) &&
              ((stringValue[2] == 's') || (stringValue[2] == 'S'))
            );
          }
          default: { // lexical_cast uses 'true'
            return Text::lexical_cast<bool>(value.value());
          }
        }
      } else {
        return std::optional<bool>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint32_t> IniSettingsStore::RetrieveUInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<std::uint32_t>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        return Text::lexical_cast<std::uint32_t>(value.value());
      } else {
        return std::optional<std::uint32_t>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int32_t> IniSettingsStore::RetrieveInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<std::int32_t>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        return Text::lexical_cast<std::int32_t>(value.value());
      } else {
        return std::optional<std::int32_t>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint64_t> IniSettingsStore::RetrieveUInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<std::uint64_t>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        return Text::lexical_cast<std::uint64_t>(value.value());
      } else {
        return std::optional<std::uint64_t>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int64_t> IniSettingsStore::RetrieveInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<std::int64_t>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        return Text::lexical_cast<std::int64_t>(value.value());
      } else {
        return std::optional<std::int64_t>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> IniSettingsStore::RetrieveStringProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    if(this->privateImplementationData == nullptr) {
      return std::optional<std::string>();
    } else {
      std::optional<std::string> value = reinterpret_cast<const IniDocumentModel *>(
        this->privateImplementationData
      )->GetPropertyValue(categoryName, propertyName);

      if(value.has_value()) {
        return value.value();
      } else {
        return std::optional<std::string>();
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreBooleanProperty(
    const std::string &categoryName, const std::string &propertyName, bool value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, Text::lexical_cast<std::string>(value)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreUInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::uint32_t value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, Text::lexical_cast<std::string>(value)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::int32_t value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, Text::lexical_cast<std::string>(value)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreUInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::uint64_t value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, Text::lexical_cast<std::string>(value)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::int64_t value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, Text::lexical_cast<std::string>(value)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void IniSettingsStore::StoreStringProperty(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
    this->modified = true;

    if(this->privateImplementationData == nullptr) {
      this->privateImplementationData = reinterpret_cast<PrivateImplementationData *>(
        new IniDocumentModel()
      );
    }
    reinterpret_cast<IniDocumentModel *>(this->privateImplementationData)->SetPropertyValue(
      categoryName, propertyName, value
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
