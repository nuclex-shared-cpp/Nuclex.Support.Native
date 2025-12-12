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

  /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
  /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
  /// <typeparam name="TConstructorSignature">
  ///   Constructor signature obtained from the constructor signature detector
  /// <typeparam>
  template<typename TImplementation, typename TConstructorSignature>
  class ServiceFactory;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
  /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
  /// <remarks>
  ///   Specialization for default-constructible types
  /// </remarks>
  template<typename TImplementation>
  class ServiceFactory<TImplementation, ConstructorSignature<>> {

    /// <summary>Creates a new instance of the service factory's type</summary>
    /// <param name="serviceProvider">Not used in this specialization</param>
    /// <returns>The new instance of the service factory's type</returns>
    public: static std::shared_ptr<TImplementation> CreateInstance(
      const std::shared_ptr<ServiceProvider> &serviceProvider
    ) {
      (void)serviceProvider; // Avoid unused variable warning
      return std::make_shared<TImplementation>();
    }

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Constructs unknown types automatically injecting their dependencies</summary>
  /// <typeparam name="TImplementation">Type that will be constructed</typeparam>
  /// <typeparam name="TConstructorSignature">
  ///   Constructor signature obtained from the constructor signature detector
  /// <typeparam>
  /// <remarks>
  ///   Variadic version for types with 1 or more constructor arguments
  /// </remarks>
  template<typename TImplementation, typename... TArguments>
  class ServiceFactory<TImplementation, ConstructorSignature<TArguments...>> {

    /// <summary>Creates a new instance of the service factory's type</summary>
    /// <param name="serviceProvider">
    ///   Service provider that will be used to resolve constructor dependencies
    /// </param>
    /// <returns>The new instance of the service factory's type</returns>
    public: static std::shared_ptr<TImplementation> CreateInstance(
      const std::shared_ptr<ServiceProvider> &serviceProvider
    ) {
      return std::make_shared<TImplementation>(
        typename TArguments::Type(serviceProvider)...
      );
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
