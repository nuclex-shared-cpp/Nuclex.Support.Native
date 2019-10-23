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

#if !defined(NUCLEX_SUPPORT_SERVICES_LAZYSERVICEINJECTOR_H)
#error This header must be included via LazyServiceInjector.h
#endif

#include <type_traits>
#include <stdexcept>

namespace Nuclex { namespace Support { namespace Services {

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>
    ///   Functor used as placeholder for constructor arguments to detect their type
    /// </summary>
    class ArgumentPlaceholder {

      /// <summary>Implicitly converts the placeholder to the argument's type</summary>
      /// <typeparam name="TArgument">The full type of the argument</typeparam>
      /// <returns>
      ///   The value that would be passed to the constructor if this was ever called
      /// </returns>
      public: template<
        typename TArgument,
        typename = typename std::enable_if<
          IsInjectableArgument<typename std::decay<TArgument>::type>::value
        >::type
      >
      operator TArgument() const {
        // TODO: Call the service injector to construct this service
        //   Use a dependency chain to detect cyclic dependencies
        return TArgument(); // We know TArgument is a std::shared_ptr()
      }

    };

  } // namespace Private

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Services
