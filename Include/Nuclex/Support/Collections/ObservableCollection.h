#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2013 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H
#define NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H

#include <cstdlib>

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides notifications for when a collection changes</summary>
  /// <remarks>
  ///   <para>
  ///     This does not monitor the state of items stored in the collection, it just reports
  ///     when the items are added, removed or replaced in the collection.
  ///   </para>
  /// </remarks>
  template<typename TValue>
  class ObservableCollection {

    // public: mutable Event ItemAdded(const TValue &value);
    // public: mutable Event ItemRemoved(const TValue &value);
    // public: mutable Event Clearing();
    // public: mutable Event Cleared();

    /// <summary>Frees all memory used by the observable collection</summary>
    public: virtual ~ObservableCollection() {}

    private: ObservableCollection(const ObservableCollection &);
    private: ObservableCollection &operator =(const ObservableCollection &);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // NUCLEX_SUPPORT_COLLECTIONS_OBSERVABLECOLLECTION_H