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

#include "./StandardServiceProvider.h"
#include "./StandardInstanceSet.h"

#include "Nuclex/Support/Errors/UnresolvedDependencyError.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::StandardServiceProvider(
    std::shared_ptr<StandardInstanceSet> &&services
  ) :
    services(services) {}

  // ------------------------------------------------------------------------------------------- //

  StandardServiceProvider::~StandardServiceProvider() = default;

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardServiceProvider::CreateScope() {
    // TODO: Implement StandardServiceProvider::CreateScope() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::TryGetService(const std::type_info &typeInfo) {
    (void)typeInfo;
    // TODO: Implement StandardServiceProvider::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardServiceProvider::GetService(const std::type_info &typeInfo) {
    const std::type_index serviceIndex(typeInfo);
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator service = (
      this->services->OwnBindings.find(serviceIndex)
    );
    if(service == this->services->OwnBindings.end()) {
      throw Errors::UnresolvedDependencyError(
        "Unknown service" // todo improve error message, use UTF-8
      );
    }

    return this->services->CreateOrFetchServiceInstance(
      std::shared_ptr<ServiceProvider>(), // TODO: use std::shared_from_this
      service
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardServiceProvider::GetServiceFactory(
    const std::type_info &typeInfo
  ) const {
    (void)typeInfo;
    // TODO: Implement StandardServiceProvider::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardServiceProvider::GetServices(const std::type_info &typeInfo) {
    (void)typeInfo;
    // TODO: Implement StandardServiceProvider::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //


} // namespace Nuclex::Support::Services2
