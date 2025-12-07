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

#if !defined(NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H)
#error This header must be included via LazyServiceInjector.h
#endif

#include <type_traits> // for std::is_class, std::is_abstract
#include <memory> // for std::shared_ptr

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Determines whether the specified argument uses std::shared_ptr</summary>
    /// <typeparam name="TChecked">Type that will be checked</typeparam>
    /// <remarks>
    ///   The default case, always 'no'
    /// </remarks>
    template<typename TChecked>
    class IsSharedPtr : public std::false_type {};

    /// <summary>Determines whether the specified argument uses std::shared_ptr</summary>
    /// <typeparam name="TChecked">Type that will be checked</typeparam>
    /// <remarks>
    ///   Specialization for std::shared_ptr types, produces 'yes'
    /// </remarks>
    template <class TChecked>
    class IsSharedPtr<std::shared_ptr<TChecked>> : public std::true_type {};

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Checks whether a constructor argument can potentially be injected</summary>
    /// <typeparam name="TArgument">Constructor argument that will be checked</typeparam>
    /// <remarks>
    ///   Any services provided by the dependency injector are wrapped in std::shared_ptr to
    ///   control the lifetime of the service implementation.
    /// </remarks>
    template<typename TArgument>
    class IsInjectableArgument : public std::integral_constant<
      bool,
      (
        std::is_class<TArgument>::value &&
        !std::is_abstract<TArgument>::value &&
        IsSharedPtr<TArgument>::value
      )
    > {};

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
