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

#if !defined(NUCLEX_SUPPORT_SERVICES2_SERVICECOLLECTION_H)
#error This header must be included via ServiceCollection.h
#endif

namespace Nuclex::Support::Services2::Private {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Checks whether a type is an <code>std::shared_ptr</code> carrying an instance
  ///   that implements a given service interface
  /// </summary>
  /// <typeparam name="TService">Service interface the instance must implement</typeparam>
  /// <typeparam name="TChecked">Type that will be checked</typeparam>
  /// <remarks>
  ///   This is the default overload that rejects any parameter that is not wrapped
  ///   in an <code>std::shared_ptr</code>.
  /// </remarks>
  template<typename TService, typename TChecked>
  struct IsServiceInstanceType : std::false_type {};

  /// <summary>
  ///   Checks whether a type is an <code>std::shared_ptr</code> carrying an instance
  ///   that implements a given service interface
  /// </summary>
  /// <typeparam name="TService">Service interface the instance must implement</typeparam>
  /// <typeparam name="TChecked">Type that will be checked</typeparam>
  /// <remarks>
  ///   Any services provided by the dependency injector are wrapped in
  ///   <code>std::shared_ptr</code> to control the lifetime of the service instance.
  /// </remarks>
  template<typename TService, typename TChecked>
  struct IsServiceInstanceType<TService, std::shared_ptr<TChecked>> :
    std::bool_constant<
      std::is_class<typename std::remove_cv<TChecked>::type>::value &&
      std::is_base_of<TService, typename std::remove_cv<TChecked>::type>::value
    > {};

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
