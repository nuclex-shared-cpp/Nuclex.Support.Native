#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_SCOPEGUARD_H
#define NUCLEX_SUPPORT_SCOPEGUARD_H

#include <type_traits>
#include <utility>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>RAII helper that executes a lambda expression when going out of scope</summary>
  /// <typeparam name="TLambda">Lambda expression that will be executed</typeparam>
  template<typename TLambda>
  class ScopeGuard {

    /// <summary>
    ///   Initializes a new scope guard running the specified expression upon destruction
    /// </summary>
    /// <param name="cleanUpExpression">
    ///   Lambda expression with the clean up code that needs to be executed
    /// </param>
    public: ScopeGuard(TLambda &&cleanUpExpression) :
      cleanUpExpression(std::forward<TLambda>(cleanUpExpression)) {}

    /// <summary>Executes the cleanup code as the scope guard leaves scope</summary>
    public: ~ScopeGuard() {
      cleanUpExpression();
    }

    /// <summary>Lambda expression containing the cleanup code</summary>
    private: TLambda cleanUpExpression;

  };

  // ------------------------------------------------------------------------------------------- //

  // Typical C-style macro to concatenate two names in the preprocessor
  #define NUCLEX_SUPPORT_CONCAT_IMPL(x, y) x##y
  #define NUCLEX_SUPPORT_CONCAT(x, y) NUCLEX_SUPPORT_CONCAT_IMPL(x, y)

  // Macro to give scope guards unique names, either sequential or line numbers
  #if defined(__COUNTER__)
    #define NUCLEX_SUPPORT_UNIQUE_VARIABLE(name) NUCLEX_SUPPORT_CONCAT(name, __COUNTER__)
  #else
    #define NUCLEX_SUPPORT_UNIQUE_VARIABLE(name) NUCLEX_SUPPORT_CONCAT(name, __LINE__)
  #endif

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_SCOPEGUARD_H
