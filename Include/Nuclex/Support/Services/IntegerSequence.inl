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

#include <cstddef>

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Vardiadic template whose arguments are a generated integer sequence</summary>
    /// <typeparam name="Integers">Sequential integers as variadic argument list</typeparam>
    template<std::size_t... Integers>
    class IntegerSequence {

      /// <summary>The type of the integer sequence</summary>
      public: typedef IntegerSequence Type;

      /// <summary>Helper that prepends another integer to the sequence</summary>
      public: template<std::size_t PrependedInteger>
      using Prepend = IntegerSequence<PrependedInteger, Integers...>;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>
    ///   Recursively constructs a variadic template whose arguments are an integer sequence
    /// </summary>
    /// <typeparam name="MaximumInteger">Largest integer (inclusive) in the sequence</typeparam>
    /// <typeparam name="TIntegerSequence">
    ///   Integer sequence constructed by previous recursions or empty of this is the first
    /// </typeparam>
    template<std::size_t MaximumInteger, typename TIntegerSequence>
    class IntegerSequenceBuilder {

      /// <summary>The recursively formed integer sequence</summary>
      /// <remarks>
      ///   This recursively defines itself all the way down until the integer is zero,
      ///   building an IntegerSequence along the way (prepending the value so
      ///   that it still produces an ascending sequence).
      /// </remarks>
      public: typedef typename IntegerSequenceBuilder<
        MaximumInteger - 1,
        typename TIntegerSequence::template Prepend<MaximumInteger>
      >::Type Type;

    };

    /// <summary>Specialization for the final recursion that just returns the sequence</summary>
    template<typename TIntegerSequence>
    class IntegerSequenceBuilder<0, TIntegerSequence> {

      /// <summary>The integer sequence the template was instantiated with</summary>
      public: typedef TIntegerSequence Type;

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>Builds variadic template whose parameters are a sequence of integers</summary>
    /// <typeparam name="MaximumInteger">
    ///   Integer up to which a sequence will be formed (inclusive)
    /// </typeparam>
    template<std::size_t MaximumInteger>
    using BuildIntegerSequence = typename IntegerSequenceBuilder<
      MaximumInteger, IntegerSequence<>
    >::Type;

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
