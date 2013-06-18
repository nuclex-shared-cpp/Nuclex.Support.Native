#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Variant.h"
#include "Nuclex/Support/Text/Lexical.h"
#include "Nuclex/Support/Text/StringConverter.h"

namespace {

  std::string InvalidVariantTypeExceptionMessage("Invalid variant type");

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  bool Variant::ToBoolean() const {
    switch(this->type) {
      case VariantType::Empty: { return false; }
      case VariantType::Boolean: { return this->booleanValue; }
      case VariantType::Uint8: { return this->uint8Value != 0; }
      case VariantType::Int8: { return this->int8Value != 0; }
      case VariantType::Uint16: { return this->uint16Value != 0; }
      case VariantType::Int16: { return this->int16Value != 0; }
      case VariantType::Uint32: { return this->uint32Value != 0; }
      case VariantType::Int32: { return this->int32Value != 0; }
      case VariantType::Uint64: { return this->uint64Value != 0; }
      case VariantType::Int64: { return this->int64Value != 0; }
      case VariantType::Float: { return this->floatValue != 0.0f; }
      case VariantType::Double: { return this->doubleValue != 0.0; }
      case VariantType::String: { return Text::lexical_cast<bool>(*this->stringValue); }
      case VariantType::WString: { return Text::wlexical_cast<bool>(*this->wstringValue); }
      case VariantType::Any: { return true; }
      case VariantType::VoidPointer: { return this->pointerValue != nullptr; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t Variant::ToUint8() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return this->uint8Value; }
      case VariantType::Int8: { return static_cast<std::uint8_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint8_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint8_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint8_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint8_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint8_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint8_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint8_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint8_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint8_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return static_cast<std::uint8_t>(
          Text::wlexical_cast<std::uint16_t>(*this->wstringValue)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::uint8_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int8_t Variant::ToInt8() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int8_t>(this->uint8Value); }
      case VariantType::Int8: { return this->int8Value; }
      case VariantType::Uint16: { return static_cast<std::int8_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int8_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int8_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int8_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int8_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int8_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int8_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int8_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int8_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return static_cast<std::int8_t>(
          Text::wlexical_cast<std::int16_t>(*this->wstringValue)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::int8_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint16_t Variant::ToUint16() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint16_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint16_t>(this->int8Value); }
      case VariantType::Uint16: { return this->uint16Value; }
      case VariantType::Int16: { return static_cast<std::uint16_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint16_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint16_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint16_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint16_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint16_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint16_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint16_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint16_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::uint16_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int16_t Variant::ToInt16() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int16_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int16_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int16_t>(this->uint16Value); }
      case VariantType::Int16: { return this->int16Value; }
      case VariantType::Uint32: { return static_cast<std::int16_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int16_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int16_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int16_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int16_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int16_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int16_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int16_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::int16_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint32_t Variant::ToUint32() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint32_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint32_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint32_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint32_t>(this->int16Value); }
      case VariantType::Uint32: { return this->uint32Value; }
      case VariantType::Int32: { return static_cast<std::uint32_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint32_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint32_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint32_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint32_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint32_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint32_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::uint32_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int32_t Variant::ToInt32() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int32_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int32_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int32_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int32_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int32_t>(this->uint32Value); }
      case VariantType::Int32: { return this->int32Value; }
      case VariantType::Uint64: { return static_cast<std::int32_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int32_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int32_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int32_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int32_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int32_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::int32_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t Variant::ToUint64() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint64_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint64_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint64_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint64_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint64_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint64_t>(this->int32Value); }
      case VariantType::Uint64: { return this->uint64Value; }
      case VariantType::Int64: { return static_cast<std::uint64_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint64_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint64_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint64_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint64_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::uint64_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int64_t Variant::ToInt64() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int64_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int64_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int64_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int64_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int64_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int64_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int64_t>(this->uint64Value); }
      case VariantType::Int64: { return this->int64Value; }
      case VariantType::Float: { return static_cast<std::int64_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int64_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int64_t>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int64_t>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return reinterpret_cast<std::int64_t>(this->pointerValue);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  float Variant::ToFloat() const {
    switch(this->type) {
      case VariantType::Empty: { return 0.0f; }
      case VariantType::Boolean: { return this->booleanValue ? 1.0f : 0.0f; }
      case VariantType::Uint8: { return static_cast<float>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<float>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<float>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<float>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<float>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<float>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<float>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<float>(this->int64Value); }
      case VariantType::Float: { return this->floatValue; }
      case VariantType::Double: { return static_cast<float>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<float>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<float>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<float>(reinterpret_cast<std::uintptr_t>(this->pointerValue));
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  double Variant::ToDouble() const {
    switch(this->type) {
      case VariantType::Empty: { return 0.0; }
      case VariantType::Boolean: { return this->booleanValue ? 1.0 : 0.0; }
      case VariantType::Uint8: { return static_cast<double>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<double>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<double>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<double>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<double>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<double>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<double>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<double>(this->int64Value); }
      case VariantType::Float: { return static_cast<double>(this->floatValue); }
      case VariantType::Double: { return this->doubleValue; }
      case VariantType::String: {
        return Text::lexical_cast<double>(*this->stringValue);
      }
      case VariantType::WString: {
        return Text::wlexical_cast<double>(*this->wstringValue);
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<double>(reinterpret_cast<std::uintptr_t>(this->pointerValue));
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::string Variant::ToString() const {
    static std::string emptyString;
    static std::string trueString("1", 1);
    static std::string falseString("0", 1);

    switch(this->type) {
      case VariantType::Empty: { return emptyString; }
      case VariantType::Boolean: { return this->booleanValue ? trueString : falseString; }
      case VariantType::Uint8: { return Text::lexical_cast<std::string>(this->uint8Value); }
      case VariantType::Int8: { return Text::lexical_cast<std::string>(this->int8Value); }
      case VariantType::Uint16: { return Text::lexical_cast<std::string>(this->uint16Value); }
      case VariantType::Int16: { return Text::lexical_cast<std::string>(this->int16Value); }
      case VariantType::Uint32: { return Text::lexical_cast<std::string>(this->uint32Value); }
      case VariantType::Int32: { return Text::lexical_cast<std::string>(this->int32Value); }
      case VariantType::Uint64: { return Text::lexical_cast<std::string>(this->uint64Value); }
      case VariantType::Int64: { return Text::lexical_cast<std::string>(this->int64Value); }
      case VariantType::Float: { return Text::lexical_cast<std::string>(this->floatValue); }
      case VariantType::Double: { return Text::lexical_cast<std::string>(this->doubleValue); }
      case VariantType::String: { return *this->stringValue; }
      case VariantType::WString: {
        return Text::StringConverter::Utf8FromWideChar(*this->wstringValue);
      }
      case VariantType::Any: { return emptyString; }
      case VariantType::VoidPointer: {
        return Text::lexical_cast<std::string>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring Variant::ToWString() const {
    static std::wstring emptyString;
    static std::wstring trueString(L"1", 1);
    static std::wstring falseString(L"0", 1);

    switch(this->type) {
      case VariantType::Empty: { return emptyString; }
      case VariantType::Boolean: { return this->booleanValue ? trueString : falseString; }
      case VariantType::Uint8: { return Text::wlexical_cast<std::wstring>(this->uint8Value); }
      case VariantType::Int8: { return Text::wlexical_cast<std::wstring>(this->int8Value); }
      case VariantType::Uint16: { return Text::wlexical_cast<std::wstring>(this->uint16Value); }
      case VariantType::Int16: { return Text::wlexical_cast<std::wstring>(this->int16Value); }
      case VariantType::Uint32: { return Text::wlexical_cast<std::wstring>(this->uint32Value); }
      case VariantType::Int32: { return Text::wlexical_cast<std::wstring>(this->int32Value); }
      case VariantType::Uint64: { return Text::wlexical_cast<std::wstring>(this->uint64Value); }
      case VariantType::Int64: { return Text::wlexical_cast<std::wstring>(this->int64Value); }
      case VariantType::Float: { return Text::wlexical_cast<std::wstring>(this->floatValue); }
      case VariantType::Double: { return Text::wlexical_cast<std::wstring>(this->doubleValue); }
      case VariantType::String: {
        return Text::StringConverter::WideCharFromUtf8(*this->stringValue);
      }
      case VariantType::WString: { return *this->wstringValue; }
      case VariantType::Any: { return emptyString; }
      case VariantType::VoidPointer: {
        return Text::wlexical_cast<std::wstring>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Any Variant::ToAny() const {
    switch(this->type) {
      case VariantType::Empty: { return Any(nullptr); }
      case VariantType::Boolean: { return Any(this->booleanValue); }
      case VariantType::Uint8: { return Any(this->uint8Value); }
      case VariantType::Int8: { return Any(this->int8Value); }
      case VariantType::Uint16: { return Any(this->uint16Value); }
      case VariantType::Int16: { return Any(this->int16Value); }
      case VariantType::Uint32: { return Any(this->uint32Value); }
      case VariantType::Int32: { return Any(this->int32Value); }
      case VariantType::Uint64: { return Any(this->uint64Value); }
      case VariantType::Int64: { return Any(this->int64Value); }
      case VariantType::Float: { return Any(this->floatValue); }
      case VariantType::Double: { return Any(this->doubleValue); }
      case VariantType::String: { return Any(*this->stringValue); }
      case VariantType::WString: { return Any(*this->wstringValue); }
      case VariantType::Any: { return this->anyValue; }
      case VariantType::VoidPointer: { return this->pointerValue; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void *Variant::ToVoidPointer() const {
    switch(this->type) {
      case VariantType::Empty: { return nullptr; }
      case VariantType::Boolean: { return reinterpret_cast<void *>(this->booleanValue); }
      case VariantType::Uint8: { return reinterpret_cast<void *>(this->uint8Value); }
      case VariantType::Int8: { return reinterpret_cast<void *>(this->int8Value); }
      case VariantType::Uint16: { return reinterpret_cast<void *>(this->uint16Value); }
      case VariantType::Int16: { return reinterpret_cast<void *>(this->int16Value); }
      case VariantType::Uint32: { return reinterpret_cast<void *>(this->uint32Value); }
      case VariantType::Int32: { return reinterpret_cast<void *>(this->int32Value); }
      case VariantType::Uint64: { return reinterpret_cast<void *>(this->uint64Value); }
      case VariantType::Int64: { return reinterpret_cast<void *>(this->int64Value); }
      case VariantType::Float: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->floatValue));
      }
      case VariantType::Double: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->doubleValue));
      }
      case VariantType::String: {
        return reinterpret_cast<void *>(Text::lexical_cast<std::uintptr_t>(this->stringValue));
      }
      case VariantType::WString: {
        return reinterpret_cast<void *>(Text::wlexical_cast<std::uintptr_t>(this->wstringValue));
      }
      case VariantType::Any: { return this->anyValue; }
      case VariantType::VoidPointer: { return this->pointerValue; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
