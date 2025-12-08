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

#ifndef NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H
#define NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Services2/ServiceScopeFactory.h"

#include <any> // for std::any
#include <vector> // for std::vector<>

namespace Nuclex::Support::Services2 {
  class ServiceProvider;
}
namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  class NUCLEX_SUPPORT_TYPE ServiceProvider : public ServiceScopeFactory {

    public: virtual ~ServiceProvider();

    public: template<typename TService> std::shared_ptr<TService> GetRequiredService();
    public: template<typename TService> std::shared_ptr<TService> GetService();
    public: template<typename TService> std::vector<std::shared_ptr<TService>> GetServices();

    public: virtual std::shared_ptr<ServiceScope> CreateScope() = 0;

    protected: virtual std::any GetRequiredService(const std::type_info &typeInfo) = 0;
    protected: virtual std::any GetService(const std::type_info &typeInfo) = 0;
    protected: virtual std::vector<std::any> GetServices(const std::type_info &typeInfo) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2

#endif // NUCLEX_SUPPORT_SERVICES2_SERVICEPROVIDER_H
