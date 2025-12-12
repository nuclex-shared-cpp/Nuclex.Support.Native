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

  /// <summary>Informations about an argument passed to the constructor of a type</summary>
  /// <typeparam name="ArgumentIndex">Index of this argument</typeparam>
  template<std::size_t ArgumentIndex>
  class ConstructorArgument {

    /// <summary>This type</summary>
    public: typedef ConstructorArgument Type;

    /// <summary>Index of this argument on the constructor</summary>
    public: constexpr static std::size_t Index = ArgumentIndex;

    /// <summary>Initializes a new constructor argument</summary>
    /// <param name="serviceProvider">
    ///   Activator through which the argument will be resolved when it is used
    /// </param>
    public: ConstructorArgument(const std::shared_ptr<ServiceProvider> &serviceProvider) :
      serviceProvider(serviceProvider) {}

    /// <summary>Implicitly converts the placeholder to the argument's type</summary>
    /// <typeparam name="TArgument">The full type of the argument</typeparam>
    /// <returns>The value that will be passed as the constructor argument</returns>
    public: template<
      typename TArgument,
      typename = typename std::enable_if<IsInjectableType<TArgument>::value>::type
    >
    operator TArgument() const {
      // IsInjectableType guarantees that TArgument is a specialization of std::shared_ptr<>
      typedef typename TArgument::element_type ServiceType;
      //return this->serviceProvider->template GetService<ServiceType>();
      throw -1;
    }

    /// <summary>Activator through which the argument will be resolved when needed</summary>
    private: std::shared_ptr<ServiceProvider> serviceProvider;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores a constructor signature (the number and type of its arguments)</summary>
  template<typename... TArguments>
  class ConstructorSignature {

    /// <summary>The type of this constructor signature itself</summary>
    public: typedef ConstructorSignature Type;

    /// <summary>Number of arguments being passed to the constructor</summary>
    public: static constexpr std::size_t ArgumentCount = sizeof...(TArguments);

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Used if the constructor signature cannot be determined</summary>
  class InvalidConstructorSignature {

    /// <summary>The type of this constructor signature itself</summary>
    public: typedef InvalidConstructorSignature Type;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
