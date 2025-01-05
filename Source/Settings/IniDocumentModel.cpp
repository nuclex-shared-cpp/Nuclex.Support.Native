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

#include "IniDocumentModel.h"
#include "IniDocumentModel.FileParser.h"

#include "Nuclex/Support/Text/ParserHelper.h"

#include <memory> // for std::unique_ptr, std::align()
#include <type_traits> // for std::is_base_of
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

// TODO: This file has become too long.
//       Split the line formatting code into a separate line builder/manager class

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the size of a type plus padding for another aligned member</summary>
  /// <typeparam name="T">Type whose size plus padding will be determined</typeparam>
  /// <returns>The size of the type plus padding with another aligned member</returns>
  template<typename T>
  constexpr std::size_t getSizePlusAlignmentPadding() {
    constexpr std::size_t misalignment = (sizeof(T) % alignof(T));
    if constexpr(misalignment > 0) {
      return sizeof(T) + (alignof(T) - misalignment);
    } else {
      return sizeof(T);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IniDocumentModel() :
    loadedLinesMemory(),
    createdLinesMemory(),
    firstLine(nullptr),
    sections(),
    hasSpacesAroundAssignment(true),
    usesPaddingLines(false),
#if defined(NUCLEX_SUPPORT_WINDOWS)
    usesCrLf(true) {}
#else
    usesCrLf(false) {}
#endif

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IniDocumentModel(const std::byte *fileContents, std::size_t byteCount) :
    loadedLinesMemory(),
    createdLinesMemory(),
    firstLine(nullptr),
    sections(),
    hasSpacesAroundAssignment(true),
    usesPaddingLines(false),
#if defined(NUCLEX_SUPPORT_WINDOWS)
    usesCrLf(true) {
#else
    usesCrLf(false) {
#endif
    parseFileContents(fileContents, byteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::~IniDocumentModel() {

    // Indexed sections need to be destructed. The memory taken by the instances will
    // be tracked in either the chunked lines memory (below) or the individual
    // created lines memory (also below).
    for(
      SectionMap::iterator iterator = this->sections.begin();
      iterator != this->sections.end();
      ++iterator
    ) {
      iterator->second->~IndexedSection();
    }

    // Delete the memory for any lines that were created by the user
    for(
      std::unordered_set<std::byte *>::iterator iterator = this->createdLinesMemory.begin();
      iterator != this->createdLinesMemory.end();
      ++iterator
    ) {
      delete[] *iterator;
    }

    // If an existing .ini file was loaded, memory will have been allocated in chunks.
    for(
      std::vector<std::byte *>::reverse_iterator iterator = this->loadedLinesMemory.rbegin();
      iterator != this->loadedLinesMemory.rend();
      ++iterator
    ) {
      delete[] *iterator;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::byte> IniDocumentModel::Serialize() const {
    std::vector<std::byte> result;

    if(this->firstLine != nullptr) {
      result.reserve(4096);
      result.insert(
        result.end(),
        this->firstLine->Contents, this->firstLine->Contents + this->firstLine->Length
      );

      Line *nextLine = this->firstLine->Next;
      while(nextLine != this->firstLine) {
        result.insert(
          result.end(),
          nextLine->Contents, nextLine->Contents + nextLine->Length
        );
        nextLine = nextLine->Next;
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t IniDocumentModel::Serialize(
    void *context, void write(void *context, const std::byte *, std::size_t)
  ) const {
    if(this->firstLine == nullptr) {
      return 0;
    } else {
      write(context, this->firstLine->Contents, this->firstLine->Length);
      std::size_t bytesWritten = this->firstLine->Length;

      Line *nextLine = this->firstLine->Next;
      while(nextLine != this->firstLine) {
        write(context, nextLine->Contents, nextLine->Length);
        bytesWritten += nextLine->Length;
        nextLine = nextLine->Next;
      }

      return bytesWritten;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniDocumentModel::GetAllSections() const {
    std::vector<std::string> sectionNames;
    sectionNames.reserve(this->sections.size());

    // If the default (global) section exists, list it first
    SectionMap::const_iterator firstSectionIterator = this->sections.find(std::string());
    if(firstSectionIterator != this->sections.end()) { // If default section exists
      sectionNames.push_back(std::string());
    }

    // Then add the remaining sections (in undefined order, thanks to unordered_map)
    for(
      SectionMap::const_iterator iterator = this->sections.begin();
      iterator != this->sections.end();
      ++iterator
    ) {
      const std::string &sectionName = iterator->first;
      if(!sectionName.empty()) { // Don't add the default (global) a second time
        sectionNames.push_back(sectionName);
      }
    }

    return sectionNames;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> IniDocumentModel::GetAllProperties(
    const std::string &sectionName
  ) const {
    SectionMap::const_iterator sectionIterator = this->sections.find(sectionName);
    if(sectionIterator == this->sections.end()) { // If section doesn't exist
      return std::vector<std::string>();
    } else { // Section exists
      const PropertyMap &properties = sectionIterator->second->Properties;

      std::vector<std::string> propertyNames;
      propertyNames.reserve(properties.size());

      for(
        PropertyMap::const_iterator propertyIterator = properties.begin();
        propertyIterator != properties.end();
        ++propertyIterator
      ) {
        propertyNames.push_back(propertyIterator->first);
      }

      return propertyNames;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> IniDocumentModel::GetPropertyValue(
    const std::string &sectionName, const std::string &propertyName
  ) const {
    SectionMap::const_iterator sectionIterator = this->sections.find(sectionName);
    if(sectionIterator == this->sections.end()) { // If section doesn't exist
      return std::optional<std::string>();
    } else { // Section exists
      const PropertyMap &properties = sectionIterator->second->Properties;
      PropertyMap::const_iterator propertyIterator = properties.find(propertyName);
      if(propertyIterator == properties.end()) { // If property doesn't exist
        return std::optional<std::string>();
      } else { // Property exists
        PropertyLine *propertyLine = propertyIterator->second;
        if(propertyLine->ValueLength > 0) { // Is value present?
          return unescape(
            propertyLine->Contents + propertyLine->ValueStartIndex,
            propertyLine->Contents + propertyLine->ValueStartIndex + propertyLine->ValueLength
          );
        } else { // Property has empty value
          return std::string();
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::SetPropertyValue(
    const std::string &sectionName,
    const std::string &propertyName,
    const std::string &propertyValue
  ) {
    IndexedSection *section = getOrCreateSection(sectionName);
    PropertyMap::iterator propertyIterator = section->Properties.find(propertyName);

    // Is this a new property? Create a new declaration line for it
    if(propertyIterator == section->Properties.end()) {
      PropertyLine *newPropertyLine = createPropertyLine(propertyName, propertyValue);
      section->Properties.insert(
        PropertyMap::value_type(propertyName, newPropertyLine)
      );
      if(section->LastLine == nullptr) { // Does section have no properties yet?
        if(section->DeclarationLine == nullptr) { // If so, is this the default section?
          if(this->firstLine == nullptr) { // Are there any lines at all?
            this->firstLine = newPropertyLine;
            newPropertyLine->Previous = newPropertyLine;
            newPropertyLine->Next = newPropertyLine;
          } else { // First line present, but property has to become new first line
            integrateLine(this->firstLine->Previous, newPropertyLine, this->usesPaddingLines);
            this->firstLine = newPropertyLine;
          }
        } else { // Section declaration line exists, put property below
          integrateLine(section->DeclarationLine, newPropertyLine, this->usesPaddingLines);
        }
      } else { // Section exists and already has properties, put new property after them
        integrateLine(section->LastLine, newPropertyLine, this->usesPaddingLines);
      }
    } else { // A property line already exists
      PropertyLine *existingPropertyLine = propertyIterator->second;
      if(existingPropertyLine->ValueStartIndex == 0) {
        PropertyLine *newPropertyLine = createPropertyLine(propertyName, propertyValue);
        newPropertyLine->Previous = existingPropertyLine->Previous;
        newPropertyLine->Next = existingPropertyLine->Next;
        existingPropertyLine->Previous->Next = newPropertyLine;
        existingPropertyLine->Next->Previous = newPropertyLine;
        propertyIterator->second = newPropertyLine;
        freeLine(existingPropertyLine);
      } else {
        bool addsQuotes = requiresQuotes(propertyValue) && !hasQuotes(existingPropertyLine);
        std::string::size_type requiredLength = propertyValue.length();
        if(addsQuotes) {
          requiredLength += 2;
        }
        if(existingPropertyLine->ValueLength >= requiredLength) { // Has enough space?
          updateExistingPropertyLine(existingPropertyLine, propertyValue, addsQuotes);
        } else {
          PropertyLine *newPropertyLine = createPropertyLine(propertyName, propertyValue);
          newPropertyLine->Previous = existingPropertyLine->Previous;
          newPropertyLine->Next = existingPropertyLine->Next;
          existingPropertyLine->Previous->Next = newPropertyLine;
          existingPropertyLine->Next->Previous = newPropertyLine;
          propertyIterator->second = newPropertyLine;
          freeLine(existingPropertyLine);
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniDocumentModel::DeleteProperty(
    const std::string &sectionName,
    const std::string &propertyName
  ) {
    SectionMap::iterator sectionIterator = this->sections.find(sectionName);
    if(sectionIterator == this->sections.end()) {
      return false;
    }

    PropertyMap &properties = sectionIterator->second->Properties;
    PropertyMap::iterator propertyIterator = properties.find(propertyName);
    if(propertyIterator == properties.end()) {
      return false;
    }

    PropertyLine *lineToRemove = propertyIterator->second;
    properties.erase(propertyIterator);

    // Unlink the line from the linked list representation of the .ini file
    lineToRemove->Previous->Next = lineToRemove->Next;
    lineToRemove->Next->Previous = lineToRemove->Previous;

    // If the removed line was the last in the section, move the last line
    // link in the section up by one (or clear it, if it was the only line)
    if(sectionIterator->second->LastLine == lineToRemove) {
      sectionIterator->second->LastLine = lineToRemove->Previous;
      if(sectionIterator->second->LastLine == lineToRemove) {
        sectionIterator->second->LastLine = nullptr;
      }
    }

    // If the removed line was the first line in the document, link the next
    // line as the first line (or clear it, if this line was the only line)
    if(this->firstLine == lineToRemove) {
      this->firstLine = lineToRemove->Next;
      if(this->firstLine == lineToRemove) {
        this->firstLine = nullptr;
      }
    }

    freeLine(lineToRemove);

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniDocumentModel::DeleteSection(const std::string &sectionName) {
    SectionMap::iterator sectionIterator = this->sections.find(sectionName);
    if(sectionIterator == this->sections.end()) {
      return false;
    }

    // Build a temporary set holding the pointers of all section-declaring lines.
    // We need this because the linked list of lines does not tag or separate section
    // declarations in any way. This is an intentional decision; building this set
    // is very fast and we have to iterate over the section's lines anyway to free them.
    std::unordered_set<Line *> sectionLines;
    {
      for(
        SectionMap::const_iterator iterator = this->sections.cbegin();
        iterator != this->sections.cend();
        ++iterator
      ) {
        sectionLines.insert(sectionIterator->second->DeclarationLine);
      }
    }

    // Eliminate all lines belonging to the section, including the section declaration.
    // This should go from the section declaration up to either the next section
    // declaration or the end of the file.
    {
      Line *startLine = sectionIterator->second->DeclarationLine;
      if(startLine == nullptr) { // If this is the default section
        startLine = this->firstLine;
      }

      // Section still may have no lines at all
      if(startLine != nullptr) {
        Line *end = startLine->Next;
        while(end != startLine) {
          if(sectionLines.find(end) != sectionLines.end()) {
            break;
          }
          if(end == this->firstLine) {
            break;
          }
          end = end->Next;
        }

        // Link the line befoe the section declaration and the first line
        // after the section is over. If this is the only section, we may
        // build a loop, of course...
        startLine->Previous->Next = end;
        end->Previous = startLine->Previous;

        while(startLine != end) {
          Line *next = startLine->Next;
          if(startLine == this->firstLine) {
            this->firstLine = next;
          }

          freeLine(startLine);
          startLine = next;
        }
      }
    }

    // Either empty the section (if it is the default section) or completely
    // remove the section and free its memory (if it was individually allocated)
    {
      if(sectionIterator->first.empty()) { // Is this the nameless default section?
        sectionIterator->second->Properties.clear();
        // Setting these to nullptr will make SetPropertyValue() insert a new
        // line at the top of the file when a property is added to this section.
        sectionIterator->second->DeclarationLine = nullptr;
        sectionIterator->second->LastLine = nullptr;
      } else { // No, this is an explicit section
        std::byte *sectionMemory = reinterpret_cast<std::byte *>(sectionIterator->second);
        this->sections.erase(sectionIterator);
        std::size_t removedElementCount = this->createdLinesMemory.erase(sectionMemory);
        if(removedElementCount > 0) {
          delete[] sectionMemory;
        }
      }
    }

    // We deleted something! Yay!
    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::IndexedSection *IniDocumentModel::getOrCreateSection(
    const std::string &sectionName
  ) {
    SectionMap::iterator sectionIterator = this->sections.find(sectionName);
    if(sectionIterator == this->sections.end()) {
      IndexedSection *newSection = allocate<IndexedSection>(0);
      new(newSection) IndexedSection();
      this->sections.insert(
        SectionMap::value_type(sectionName, newSection)
      );

      // Can the default section at the start of the file be used for this?
      if(sectionName.empty()) {

        // Caller *must* place new properties at beginning of file when
        // LastLine and DeclarationLine are both nullptr.
        newSection->DeclarationLine = nullptr;
        newSection->LastLine = nullptr;
        return newSection;

      } else { // Section has a name, explicit declaration needed
        std::string::size_type nameLength = sectionName.length();
        SectionLine *newDeclarationLine = allocateLine<SectionLine>(
          nullptr, nameLength + (this->usesCrLf ? 4 : 3)
        );

        newDeclarationLine->Contents[0] = static_cast<std::byte>('[');
        std::copy_n(
          reinterpret_cast<const std::byte *>(sectionName.c_str()),
          nameLength,
          newDeclarationLine->Contents + 1
        );
        newDeclarationLine->Contents[nameLength + 1] = static_cast<std::byte>(']');
        if(this->usesCrLf) {
          newDeclarationLine->Contents[nameLength + 2] = static_cast<std::byte>('\r');
          newDeclarationLine->Contents[nameLength + 3] = static_cast<std::byte>('\n');
        } else {
          newDeclarationLine->Contents[nameLength + 2] = static_cast<std::byte>('\n');
        }

        newDeclarationLine->NameStartIndex = 1;
        newDeclarationLine->NameLength = nameLength;

        if(this->firstLine != nullptr) {
          integrateLine(this->firstLine->Previous, newDeclarationLine, true);
        } else {
          Line *blankLine = allocateLine<Line>(nullptr, (this->usesCrLf ? 2 : 1));
          if(this->usesCrLf) {
            blankLine->Contents[0] = static_cast<std::byte>('\r');
            blankLine->Contents[1] = static_cast<std::byte>('\n');
          } else {
            blankLine->Contents[0] = static_cast<std::byte>('\n');
          }

          this->firstLine = blankLine;

          blankLine->Next = newDeclarationLine;
          blankLine->Previous = newDeclarationLine;
          newDeclarationLine->Previous = blankLine;
          newDeclarationLine->Next = blankLine;
        }

        newSection->DeclarationLine = newDeclarationLine;
        newSection->LastLine = newDeclarationLine;
        return newSection;
      }
    } else { // Way at the beginning of this method, a section was found
      return sectionIterator->second;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  IniDocumentModel::PropertyLine *IniDocumentModel::createPropertyLine(
    const std::string &propertyName, const std::string &propertyValue
  ) {
    bool requiresQuotes = IniDocumentModel::requiresQuotes(propertyValue);

    // Generate a new property declaration line
    PropertyLine *newPropertyLine = allocateLine<PropertyLine>(
      nullptr,
      (
        propertyName.length() + getSerializedLength(propertyValue) +
        (this->hasSpacesAroundAssignment ? 3 : 1) +
        (this->usesCrLf ? 2 : 1) +
        (requiresQuotes ? 2 : 0)
      )
    );

    // Add the property name to the line
    {
      newPropertyLine->NameStartIndex = 0;
      newPropertyLine->NameLength = newPropertyLine->Length = propertyName.length();
      std::copy_n(
        reinterpret_cast<const std::byte *>(propertyName.c_str()),
        newPropertyLine->NameLength,
        newPropertyLine->Contents
      );
    }

    // Add an equals sign after the property name
    if(this->hasSpacesAroundAssignment) {
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>(' ');
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('=');
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>(' ');
    } else {
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('=');
    }

    // Write the value of the property behind the equals sign
    {
      if(requiresQuotes) {
        newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('"');
      }

      newPropertyLine->ValueStartIndex = newPropertyLine->Length;
      newPropertyLine->ValueLength = escape(
        newPropertyLine->Contents + newPropertyLine->Length,
        propertyValue.c_str(),
        propertyValue.length()
      );
      newPropertyLine->Length += newPropertyLine->ValueLength;

      if(requiresQuotes) {
        newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('"');
      }
    }

    // Add a line break at the end of the line
    if(this->usesCrLf) {
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('\r');
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('\n');
    } else {
      newPropertyLine->Contents[newPropertyLine->Length++] = static_cast<std::byte>('\n');
    }

    return newPropertyLine;
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::integrateLine(
    Line *previous, Line *newLine, bool extraBlankLineBefore /* = false */
  ) {
    if(extraBlankLineBefore) {
      Line *blankLine = allocateLine<Line>(nullptr, (this->usesCrLf ? 2 : 1));
      if(this->usesCrLf) {
        blankLine->Contents[0] = static_cast<std::byte>('\r');
        blankLine->Contents[1] = static_cast<std::byte>('\n');
      } else {
        blankLine->Contents[0] = static_cast<std::byte>('\n');
      }

      blankLine->Previous = previous;
      blankLine->Next = newLine;

      newLine->Previous = blankLine;
      newLine->Next = previous->Next;

      previous->Next->Previous = newLine;
      previous->Next = blankLine;
    } else {
      newLine->Previous = previous;
      newLine->Next = previous->Next;

      previous->Next->Previous = newLine;
      previous->Next = newLine;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::parseFileContents(
    const std::byte *fileContents, std::size_t byteCount
  ) {
    FileParser parser(fileContents, byteCount);
    parser.ParseInto(this);

    this->usesCrLf = parser.UsesCarriageReturns();
    this->hasSpacesAroundAssignment = parser.UsesSpacesAroundAssignment();
    this->usesPaddingLines = parser.UsesBlankLines();
  }

  // ------------------------------------------------------------------------------------------- //

  void IniDocumentModel::updateExistingPropertyLine(
    PropertyLine *line, const std::string &newValue, bool addQuotes
  ) {

    // Number of bytes from the end of the value to the end of the line
    std::string::size_type remainderStartIndex = line->ValueStartIndex + line->ValueLength;
    std::string::size_type remainderLength = line->Length - remainderStartIndex;

    // Write the new property value over the old one (and add quotes if required)
    std::byte *writeStart = (line->Contents + line->ValueStartIndex);
    {
      if(addQuotes) {
        *writeStart = static_cast<std::byte>('"');
        ++writeStart;
        ++line->ValueStartIndex;
      }

      writeStart += escape(writeStart, newValue.c_str(), newValue.length());

      if(addQuotes) {
        *writeStart = static_cast<std::byte>('"');
        ++writeStart;
      }
    }

    // Drag whatever was behind the old value along. Usually it's just the chosen newline
    // character, but any spaces and comments will be likewise kept this way.
    std::copy_n(
      line->Contents + remainderStartIndex,
      remainderLength,
      writeStart
    );
    writeStart += remainderLength;

    line->ValueLength = newValue.length();
    line->Length = writeStart - line->Contents;

  }

  // ------------------------------------------------------------------------------------------- //

  bool IniDocumentModel::hasQuotes(PropertyLine *propertyLine) {
    if(propertyLine->ValueStartIndex == 0) {
      return false;
    }

    std::byte before = propertyLine->Contents[propertyLine->ValueStartIndex - 1];
    return (before == static_cast<std::byte>('"'));
  }

  // ------------------------------------------------------------------------------------------- //

  bool IniDocumentModel::requiresQuotes(const std::string &propertyValue) {
    std::string::size_type length = propertyValue.length();
    if(length > 0) {
      bool startsOrEndsWithSpace = (
        Text::ParserHelper::IsWhitespace(propertyValue[0]) ||
        Text::ParserHelper::IsWhitespace(propertyValue[length - 1])
      );
      if(startsOrEndsWithSpace) {
        return true;
      }
      for(std::string::size_type index = 0; index < length; ++index) {
        char current = propertyValue[index];
        if((current == '"') || (current == '=') || (current == '\n')) {
          return true;
        }
      }
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string::size_type IniDocumentModel::getSerializedLength(const std::string &propertyValue) {
    std::string::size_type serializedLength = 0;

    std::string::size_type length = propertyValue.length();
    for(std::string::size_type index = 0; index < length; ++index) {
      switch(propertyValue[index]) {
        case '\\':
        case '"': { serializedLength += 2; break; }
        default: { ++serializedLength; break; }
      }
    }

    return serializedLength;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string::size_type IniDocumentModel::escape(
    std::byte *target, const char *source, std::string::size_type length
  ) {
    std::string::size_type targetIndex = 0;

    for(std::string::size_type sourceIndex = 0; sourceIndex < length; ++sourceIndex) {
      switch(source[sourceIndex]) {
        case '\\': {
          target[targetIndex] = static_cast<std::byte>('\\');
          ++targetIndex;
          target[targetIndex] = static_cast<std::byte>('\\');
          ++targetIndex;
          break;
        }
        case '"': {
          target[targetIndex] = static_cast<std::byte>('\\');
          ++targetIndex;
          target[targetIndex] = static_cast<std::byte>('\"');
          ++targetIndex;
          break;
        }
        default: {
          target[targetIndex] = static_cast<std::byte>(source[sourceIndex]);
          ++targetIndex;
          break;
        }
      } // switch on source character
    } // for each source character

    return targetIndex;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string IniDocumentModel::unescape(const std::byte *begin, const std::byte *end) {
    std::string result;
    result.reserve(end - begin); // Should be a very close guess for the line's length

    bool escapeMode = false;
    while(begin < end) {
      if(escapeMode) {
        result.push_back(static_cast<char>(*begin));
        escapeMode = false;
      } else if(*begin == static_cast<std::byte>('\\')) {
        escapeMode = true;
      } else {
        result.push_back(static_cast<char>(*begin));
      }

      ++begin;
    }

    // We'll treat an open-ended backslash at the end of an unquoted property value
    // as a simple backslash. There is a convention to treat a backslash at the end of
    // a line as a continuation (i.e. continue after line break), but I haven't ever
    // seen that convention used in .ini files. Principle of least surprise and all.
    if(escapeMode) {
      result.push_back('\\');
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  TLine *IniDocumentModel::allocateLine(const std::byte *contents, std::size_t byteCount) {
    static_assert(std::is_base_of<Line, TLine>::value, u8"TLine must inherit from Line");

    // Allocate memory for a new line, assign its content pointer to hold
    // the line loaded from the .ini file and copy the line contents into it.
    TLine *newLine = allocate<TLine>(byteCount);
    {
      newLine->Contents = (
        reinterpret_cast<std::byte *>(newLine) + getSizePlusAlignmentPadding<TLine>()
      );
      newLine->Length = byteCount;

      if(contents != nullptr) {
        std::copy_n(contents, byteCount, newLine->Contents);
      }
    }

    return newLine;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename T>
  T *IniDocumentModel::allocate(std::size_t extraByteCount /* = 0 */) {

    // While we're asked to allocate a specific type, making extra bytes available
    // requires us to allocate as std::uint8_t. The start address still needs to be
    // appropriately aligned for the requested type (otherwise we'd have to keep
    // separate pointers for delete[] and for the allocated type).
    #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(T));
    #endif

    // Calculate the exact amount of memory required, including the extra bytes
    // aligned to the same conditions as the requested type.
    constexpr std::size_t requiredMemory = getSizePlusAlignmentPadding<T>();
    std::byte *bytes = new std::byte[requiredMemory + extraByteCount];
    this->createdLinesMemory.insert(bytes);

    return reinterpret_cast<T *>(bytes);

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TLine>
  void IniDocumentModel::freeLine(TLine *line) {
    static_assert(std::is_base_of<Line, TLine>::value, u8"TLine must inherit from Line");

    std::byte *bytes = reinterpret_cast<std::byte *>(line);
    std::unordered_set<std::byte *>::iterator iterator = (
      this->createdLinesMemory.find(bytes)
    );
    if(iterator == this->createdLinesMemory.end()) {
      // Do nothing, line was chunk-allocated and is tracked in loadedLinesMemory
    } else {
      this->createdLinesMemory.erase(iterator);
      delete[] bytes;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings
