#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2012 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_THREADING_MEMORYBARRIER_H
#define NUCLEX_SUPPORT_THREADING_MEMORYBARRIER_H

// This is not how the design will look in the end. Don't use.
#if 0

#if defined(_MSC_VER)

  // ------------------------------------------------------------------------------------------- //

  #include <intrin.h>
  #define WIN32_LEAN_AND_MEAN
  #define VC_EXTRALEAN
  #include <Windows.h>

  #pragma intrinsic(_ReadWriteBarrier)
  #pragma intrinsic(_WriteBarrier)
  #pragma intrinsic(_ReadBarrier)

  // MemoryBarrier() emits the CPU fence instruction, the intrinsics force Visual C++
  // to not reorder instructions across the barrier.

  #define NUCLEX_STORAGE_MEMORY_BARRIER { _ReadWriteBarrier(); MemoryBarrier(); }
  #define NUCLEX_STORAGE_READ_MEMORY_BARRIER { _ReadBarrier(); MemoryBarrier(); }
  #define NUCLEX_STORAGE_WRITE_MEMORY_BARRIER { _WriteBarrier(); MemoryBarrier(); }

  // ------------------------------------------------------------------------------------------- //

#else

  // ------------------------------------------------------------------------------------------- //

  #error Please provide your compiler's memory barrier implementation here
  #define NUCLEX_STORAGE_MEMORY_BARRIER {}
  #define NUCLEX_STORAGE_READ_MEMORY_BARRIER {}
  #define NUCLEX_STORAGE_WRITE_MEMORY_BARRIER {}

  // ------------------------------------------------------------------------------------------- //

#endif

#endif

#endif // NUCLEX_SUPPORT_THREADING_MEMORYBARRIER_H
