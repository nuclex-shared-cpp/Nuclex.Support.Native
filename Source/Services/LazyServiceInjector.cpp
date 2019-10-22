#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#include "Nuclex/Support/Services/LazyServiceInjector.h"

#include <stdexcept>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  const Any &LazyServiceInjector::Get(const std::type_info &serviceType) const {
    Any service;

    bool alreadyCreated = this->services.TryGet(serviceType, service);
    if(alreadyCreated) {
      return this->services.Get(serviceType); // TODO: Find more efficient ServiceProvider design
    }

    throw std::runtime_error("Service creation is not implemented yet :-(");
  }

  // ------------------------------------------------------------------------------------------- //

  bool LazyServiceInjector::TryGet(const std::type_info &serviceType, Any &service) const {
    if(this->services.TryGet(serviceType, service)) {
      return true;
    }

    throw std::runtime_error("Service creation is not implemented yet :-(");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
