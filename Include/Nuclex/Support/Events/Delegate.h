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
  ///   <para>
  ///     A delegate is in principle a glorified function pointer, one that can invoke plain
  ///     free functions as well as object methods (capturing the 'this' pointer of the instance
  ///     the method is to be called on) or lamba expressions.
  ///   </para>
  ///   <para>
  ///     If you're up-to-speed in modern C++, it's essentially like std::function with
  ///     a little bit of std::bind mixed in for the 'this' pointer capture. But unlike
  ///     std::function, it is identity-comparable (i.e. you can check if two delegates are
  ///     invoking the exact same free function, object method or lambda expression).
  ///   </para>
  ///   <para>
  ///     This makes delegates useful to implement subscriptions in publisher/subscriber
  ///     systems (aka. signals/slots) that can be unregistered without magic handles.
  ///   </para>
  /// </remarks>
  template<typename TResult, typename... TArguments>
  class Delegate<TResult(TArguments...)> {

    /// <summary>Type of value that will be returned by the delegate</summary>
    public: typedef TResult ResultType;
    /// <summary>Method signature for the callbacks notified through this event</summary>
    public: typedef TResult CallType(TArguments...);

    /// <summary>Creates a delegate that will invoke the specified free function</summary>
    /// <typeparam name="TMethod">Free function that will be called by the delegate</typeparam>
    /// <returns>A delegate that invokes the specified free</returns>
    public: template<TResult(*TMethod)(TArguments...)>
    static Delegate FromFreeFunction() {
      Delegate result;
      result.instance = nullptr;
      result.method = &Delegate::callFreeFunction<TMethod>;
      return result;
    }

    /// <summary>Creates a delegate that will invoke the specified free function</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TMethod">Free function that will be called by the delegate</typeparam>
    /// <returns>A delegate that invokes the specified free</returns>
    public: template<typename TClass, TResult(TClass::*TMethod)(TArguments...)>
    static Delegate FromObjectMethod(TClass *instance) {
      Delegate result;
      result.instance = reinterpret_cast<void *>(instance);
      result.method = &Delegate::callObjectMethod<TClass, TMethod>;
      return result;
    }

    /// <summary>Initializes a new delegate as copy of an existing delegate</summary>
    /// <param name="other">Existing delegate that will be copied</param>
    public: Delegate(const Delegate &other) = default;

    /// <summary>Initializes a new delegate by taking over an existing delegate</summary>
    /// <param name="other">Existing delegate that will be taken over</param>
    public: Delegate(Delegate &&other) = default;

    /// <summary>Frees all resources owned by the delegate</summary>
    public: ~Delegate() = default;

    /// <summary>Constructs an uninitialized delegate, for internal use only<summary>
    private: Delegate() = default;

    /// <summary>Invokes the delegate</summary>
    /// <param name="arguments">Arguments as defined by the call signature</param>
    /// <returns>The value returned by the called delegate, if any</returns>
    public: TResult operator()(TArguments... arguments) {
      return (this->*method)(arguments...);
    }

    /// <summary>Makes this delegate a copy of another delegate</summary>
    /// <param name="other">Other delegate that will be copied</param>
    /// <returns>This delegate</returns>
    public: Delegate &operator =(const Delegate &other) = default;

    /// <summary>Lets this delegate take over another delegate</summary>
    /// <param name="other">Other delegate that will be taken over</param>
    /// <returns>This delegate</returns>
    public: Delegate &operator =(Delegate &&other) = default;

    /// <summary>Type of the call wrappers that invoke the target method</summary>
    private: typedef TResult (Delegate::*CallWrapperType)(TArguments...);

    /// <summary>Call wrapper that invokes a free function</summary>
    /// <typeparam name="TFreeFunction">Function that will be invoked</typeparam>
    private: template<TResult(*TFreeFunction)(TArguments...)>
    TResult callFreeFunction(TArguments... arguments) {
      return (TFreeFunction)(arguments...);
    }

    /// <summary>Call wrapper that invokes a free function</summary>
    /// <typeparam name="TClass">Class the object method is a member of</typeparam>
    /// <typeparam name="TFreeFunction">Function that will be invoked</typeparam>
    private: template<typename TClass, TResult(TClass::*TObjectMethod)(TArguments...)>
    TResult callObjectMethod(TArguments... arguments) {
      TClass *typedInstance = reinterpret_cast<TClass *>(this->instance);
      return (typedInstance->*TObjectMethod)(arguments...);
    }

    /// <summary>Instance on which the callback will take place, if applicable<summary>
    private: void *instance;
    /// <summary>Address of the call wrapper that will call the subscribed method</summary>
    private: CallWrapperType method;

  };

  // ------------------------------------------------------------------------------------------- //

  // Add construction methods akin to std::make_pair(), std::make_shared() etc. here

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Events

#endif // NUCLEX_SUPPORT_EVENTS_DELEGATE_H
