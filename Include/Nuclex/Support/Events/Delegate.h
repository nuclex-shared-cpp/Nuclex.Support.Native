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

#ifndef NUCLEX_SUPPORT_EVENTS_DELEGATE_H
#define NUCLEX_SUPPORT_EVENTS_DELEGATE_H

namespace Nuclex { namespace Support { namespace Events {

  // ------------------------------------------------------------------------------------------- //

  // Required to declare the type
  template <typename TResult> class Delegate;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Callback to a free function, method or lambda expression</summary>
  /// <typeparam name="TResult">Type that will be returned from the method</typeparam>
  /// <typeparam name="TArguments">Types of the arguments accepted by the callback</typeparam>
  /// <remarks>
  ///   This is essentially the same as std::function (and a bit of std::bind for this pointer
  ///   capture), but designed for 
  template<typename TResult, typename... TArguments>
  class Delegate<TResult(TArguments...)> {

    /// <summary>Type of value that will be returned by event subscribers</summary>
    //public: typedef TResult ReturnType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    //public: typedef TResult CallbackType(TArguments...);
    /// <summary>Type that is returned by the delegate</summary>
    //public: typedef TResult ReturnType;
    //using Thunk = RT(*)(void*, Args&&...);
    //public: typedef TResult(*)(TArguments...) FreeFunctionType;

    using CallWrapperPointerType = TResult(*)(void *instance, TArguments...);

    public: template <TResult(*TMethod)(TArguments...)>
    static Delegate ToFreeFunction() {
      Delegate result;
      result.instance = nullptr;
      result.method = CallFreeFunction<TMethod>;
      return result;
    }

    // This can be made more efficient, I believe. Store a pointer to a method
    // of /this/ class, a clever compiler could just do a single 'jmp' without
    // massaging function prolog/epilog or parameters in the slightest.
    public: TResult operator()(TArguments... arguments) const {
      return (*method)(instance, arguments...);
    }

    /// <summary>Call wrapper that invokes a free function</summary>
    template <TResult(*TMethod)(TArguments...)>
    static TResult CallFreeFunction(void *, TArguments... arguments) {
      return (TMethod)(arguments...);
    }

    /// <summary>Instance on which the callback will take place, if applicable<summary>
    private: void *instance;
    /// <summary>Address of the call wrapper that will call the subscribed method</summary>
    private: CallWrapperPointerType method;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_DELEGATE_H
