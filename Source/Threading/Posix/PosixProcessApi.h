#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_THREADING_WINDOWS_POSIXPROCESSAPI_H
#define NUCLEX_SUPPORT_THREADING_WINDOWS_POSIXPROCESSAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "../../Helpers/PosixApi.h"

namespace Nuclex { namespace Support { namespace Threading { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Windows process and inter-process communication API</summary>
  class WindowsProcessApi {


  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Posix

#endif // defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_THREADING_WINDOWS_POSIXPROCESSAPI_H
