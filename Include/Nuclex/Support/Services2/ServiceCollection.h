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

#ifndef NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H
#define NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H

#include "Nuclex/Support/Config.h"

#include <memory> // for std::shared_ptr
#include <type_traits> // for std::decay, std::type_info
#include <any> // for std::any

namespace Nuclex::Support::Services2 {
  class ServiceProvider;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  class NUCLEX_SUPPORT_TYPE ServiceCollection {

    public: ServiceCollection();
    public: ~ServiceCollection();

    public: std::shared_ptr<ServiceProvider> BuildServiceProvider();

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H
