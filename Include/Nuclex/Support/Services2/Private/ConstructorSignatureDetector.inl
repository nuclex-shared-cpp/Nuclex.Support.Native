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

#include <cstddef>
#include <utility>

namespace Nuclex::Support::Services2::Private {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Detects the constructor signature for the specified type</summary>
  /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
  template<typename TImplementation, std::size_t ArgumentCount, typename = void>
  struct ConstructorSignatureDetector {
    /// <summary>Default should that never be hit, added just in case</summary>
    typedef InvalidConstructorSignature Type;
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Helper that uses pack of integer arguments to form a pack of
  ///   <see cref="ConstructorArgument" /> types that become stand-ins for
  ///   the actual constructor arguments when the factory method is generated
  /// </summary>
  template<typename TImplementation, typename TIndexSequence>
  struct ConstructorSignatureDetectorWithIndices;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Detects the constructor signature for the specified type</summary>
  /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
  /// <typeparam name="ArgumentCount">
  ///   Number of constructor arguments to start checking with (increased via template
  ///   recursion until finding the correct number or hitting the upper limit)
  /// <typeparam>
  /// <remarks>
  ///   When the upper limit is hit, this variant is rejected via SFINAE.
  /// </remarks>
  template<typename TImplementation, std::size_t ArgumentCount>
  struct ConstructorSignatureDetector<
    TImplementation,
    ArgumentCount,
    typename std::enable_if<(MaximumConstructorArgumentCount >= ArgumentCount)>::type
  > {
    /// <summary>The detected consructor signature</summary>
    /// <remarks>
    ///   this stores either the constructor signature (as a type holding a pack of
    ///   <see cref="ConstructorArgument" /> instances matching the number of constructor
    ///   parameters) or a special type indicating that the constructor signature is invalid.
    /// </remarks>
    public: typedef typename ConstructorSignatureDetectorWithIndices<
      TImplementation, std::make_index_sequence<ArgumentCount>
    >::Type Type;
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Provides the constructor signature with a pack of <see cref="ConstructorArgument" />
  ///   types for factory method generation or uses template recursion to check for
  ///   a constructor with one more argument if the parameters didn't match
  /// </summary>
  /// <typeparam name="TImplementation">
  ///   Concrete class whose constructor is being detected
  /// </typeparam>
  /// <typeparam name="ArgumentIndices">
  ///   Pack containing an integer sequence that keeps the constructor argument stand-ins
  ///   as separate types
  /// </typeparam>
  template<typename TImplementation, std::size_t... ArgumentIndices>
  struct ConstructorSignatureDetectorWithIndices<
    TImplementation,
    std::index_sequence<ArgumentIndices...>
  > {
    /// <summary>
    ///   The constructor signature or, if detection failed, the special type that
    ///   indicates that the signature is invalid
    /// </summary>
    public: typedef typename std::conditional<
      std::is_constructible<TImplementation, ConstructorArgument<ArgumentIndices>...>::value,
      // If constructible, build the constructor signature with the argument pack
      ConstructorSignature<ConstructorArgument<ArgumentIndices>...>,
      // IF not constructible, check with one more argument
      typename ConstructorSignatureDetector<
        TImplementation, sizeof...(ArgumentIndices) + 1
      >::Type
    >::type Type;
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Special 'cap' that is selected once template recursion arrived at the maximum
  ///   number of constructor arguments that should be tried when detecting
  ///   the constructor signature of an arbitrary type
  /// </summary>
  /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
  /// <typeparam name="ArgumentCount">
  ///   Number of constructor arguments to start checking with (increased via template
  ///   recursion until finding the correct number or hitting the upper limit)
  /// <typeparam>
  /// <remarks>
  ///   This class has a another variant above that is normally used to actually check
  ///   constructor parameters. That variant is deselected via SFINAE when the parameter
  ///   count limit is hit to prevent an endless template recursion.
  /// </remarks>
  template<typename TImplementation>
  struct ConstructorSignatureDetector<
    TImplementation,
    MaximumConstructorArgumentCount + 1,
    void
  > {
    /// <summary>
    ///   Indicate constructor signature detection has failed by making the result
    ///   the special type that indicates the signature is invalid
    /// </summary>
    public: typedef InvalidConstructorSignature Type;
  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2::Private
