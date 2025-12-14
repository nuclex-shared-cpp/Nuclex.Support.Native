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

#if !defined(NUCLEX_SUPPORT_SERVICES_SERVICECOLLECTION_H)
#error This header must be included via ServiceCollection.h
#endif

namespace Nuclex::Support::Services::Private {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines whether the specified type is a std::shared_ptr of any specialization
  /// </summary>
  /// <typeparam name="TChecked">Type that will be checked</typeparam>
  /// <remarks>
  ///   The default case, always 'false'
  /// </remarks>
  template<typename TChecked>
  struct IsSharedPtr : public std::false_type {};

  /// <summary>
  ///   Determines whether the specified type is a std::shared_ptr of any specialization
  /// </summary>
  /// <typeparam name="TChecked">Type that will be checked</typeparam>
  /// <remarks>
  ///   Specialization for std::shared_ptr types, produces 'true'
  /// </remarks>
  template <class TChecked>
  struct IsSharedPtr<std::shared_ptr<TChecked>> : public std::true_type {};

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services::Private
