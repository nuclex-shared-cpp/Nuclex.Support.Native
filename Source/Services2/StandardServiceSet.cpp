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

#include "./StandardServiceSet.h"

#include <stdexcept> // for std::runtime_error()

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding::Binding(
    const std::function<std::any(const std::shared_ptr<ServiceProvider> &)> &factory
  ) :
    UniqueServiceIndex(0),
    ProvidedInstance(), // leave empty
    Factory(factory) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding::Binding(
    const std::any &providedInstance,
    const std::function<std::any(const std::any &)> &cloneFactory
  ) :
    UniqueServiceIndex(0),
    ProvidedInstance(providedInstance),
    CloneFactory(cloneFactory) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding::Binding(const Binding &other) :
    UniqueServiceIndex(0),
    ProvidedInstance(other.ProvidedInstance) {

    if(other.ProvidedInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    } else {
      new(&this->Factory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding::Binding(Binding &&other) :
    UniqueServiceIndex(0),
    ProvidedInstance(std::move(other.ProvidedInstance)) {

    if(other.ProvidedInstance.has_value()) {
      new(&this->CloneFactory) std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    } else {
      new(&this->Factory) std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding::~Binding() {
    if(this->ProvidedInstance.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding &StandardServiceSet::Binding::operator =(
    const Binding &other
  ) {
    if(this->ProvidedInstance.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }

    this->ProvidedInstance = other.ProvidedInstance;

    if(other.ProvidedInstance.has_value()) {
      new(&this->CloneFactory) std::function<std::any(const std::any &)>(
        other.CloneFactory
      );
    } else {
      new(&this->Factory) std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        other.Factory
      );
    }

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  StandardServiceSet::Binding &StandardServiceSet::Binding::operator =(
    Binding &&other
  ) {
    if(this->ProvidedInstance.has_value()) {
      this->CloneFactory.~function();
    } else {
      this->Factory.~function();
    }

    this->ProvidedInstance = std::move(other.ProvidedInstance);

    if(other.ProvidedInstance.has_value()) {
      new(&this->CloneFactory)std::function<std::any(const std::any &)>(
        std::move(other.CloneFactory)
      );
    } else {
      new(&this->Factory)std::function<std::any(const std::shared_ptr<ServiceProvider> &)>(
        std::move(other.Factory)
      );
    }

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  void StandardServiceSet::GenerateUniqueIndexes() {
    // TODO: Fill the unique index member of each service in the singleton and scoped map
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2
