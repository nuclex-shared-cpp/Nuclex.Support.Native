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

#include <cstddef> // for std::size_t
#include <type_traits> // for std::enable_if, std::is_constructible

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <typeparam name="TArgumentSequence">
    ///   Integer sequence of length matching the argument count
    /// </typeparam>
    /// <remarks>
    ///   <para>
    ///     This uses SFINAE to pick between two implementations, one that inherits from
    ///     the <see cref="ConstructorSignature" /> type if the argument count matches
    ///     and one that inherits from another constructor detector with N + 1 arguments.
    ///   </para>
    ///   <para>
    ///     So if you have a type with 2 injectable arguments, there'll be a constructor
    ///     detector for default-constructible types, inheriting from a constructor detector
    ///     for 1 argument, inheriting from a construftor detector for 2 arguments, finally
    ///     inheriting from a <see cref="ConstructorSignature" /> for 2 arguments.
    ///   </para>
    /// </remarks>
    template<typename TImplementation, typename TArgumentSequence, typename = void>
    class ConstructorSignatureDetector;

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Starting try, used if the type is default-constructible
    /// </remarks>
    template<typename TImplementation>
    class ConstructorSignatureDetector<
      TImplementation,
      IntegerSequence<>,
      typename std::enable_if<
        std::is_constructible<TImplementation>::value
      >::type
    > : public ConstructorSignature<> {};

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Starting try, delegates to check with 1 argument if type is not default-constructible
    /// </remarks>
    template<typename TImplementation>
    class ConstructorSignatureDetector <
      TImplementation,
      IntegerSequence<>,
      typename std::enable_if<
        !std::is_constructible<TImplementation>::value
      >::type
    > : public ConstructorSignatureDetector<TImplementation, BuildIntegerSequence<1>>::Type {};

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Intermediate successful attempt, used if the argument count matches
    /// </remarks>
    template<typename TImplementation, std::size_t... TArgumentIndices>
    class ConstructorSignatureDetector <
      TImplementation,
      IntegerSequence<TArgumentIndices...>,
      typename std::enable_if<
        (sizeof...(TArgumentIndices) > 0) &&
        (sizeof...(TArgumentIndices) < MaximumConstructorArgumentCount) &&
        std::is_constructible<
          TImplementation, ConstructorArgument<TArgumentIndices>...
        >::value
      >::type
    > : public ConstructorSignature<ConstructorArgument<TArgumentIndices>...> {};

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Intermediate failed attempt, delegates recursively to check with N + 1 arguments
    /// </remarks>
    template<typename TImplementation, std::size_t... TArgumentIndices>
    class ConstructorSignatureDetector<
      TImplementation,
      IntegerSequence<TArgumentIndices...>,
      typename std::enable_if<
        (sizeof...(TArgumentIndices) > 0) &&
        (sizeof...(TArgumentIndices) < MaximumConstructorArgumentCount) &&
        !std::is_constructible<
          TImplementation, ConstructorArgument<TArgumentIndices>...
        >::value
      >::type
    > : public ConstructorSignatureDetector<
      TImplementation, BuildIntegerSequence<sizeof...(TArgumentIndices) + 1>
    >::Type {};

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Last attempt, used if the argument count matches the maximum number of arguments
    /// </remarks>
    template<typename TImplementation, std::size_t... TArgumentIndices>
    class ConstructorSignatureDetector<
      TImplementation,
      IntegerSequence<TArgumentIndices...>,
      typename std::enable_if<
        (sizeof...(TArgumentIndices) == MaximumConstructorArgumentCount) &&
        std::is_constructible<
          TImplementation, ConstructorArgument<TArgumentIndices>...
        >::value
      >::type
    > : public ConstructorSignature<ConstructorArgument<TArgumentIndices>...> {};

    /// <summary>Detects the constructor signature of the specified type</summary>
    /// <typeparam name="TImplementation">Type whose constructor will be detected</typeparam>
    /// <remarks>
    ///   Last attempt, also failed, inherits from an invalid signature marker type
    /// </remarks>
    template<typename TImplementation, std::size_t... TArgumentIndices>
    class ConstructorSignatureDetector<
      TImplementation,
      IntegerSequence<TArgumentIndices...>,
      typename std::enable_if<
        (sizeof...(TArgumentIndices) == MaximumConstructorArgumentCount) &&
        !std::is_constructible<
          TImplementation, ConstructorArgument<TArgumentIndices>...
        >::value
      >::type
    > : public InvalidConstructorSignature {};

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Detects the constructor signature for the specified type</summary>
    /// <typeparam name="TImplementation">
    ///   Type for which the constructor signature will be detectd
    /// </typeparam>
    template<typename TImplementation>
    using DetectConstructorSignature = typename ConstructorSignatureDetector<
      TImplementation, BuildIntegerSequence<0>
    >::Type;

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
