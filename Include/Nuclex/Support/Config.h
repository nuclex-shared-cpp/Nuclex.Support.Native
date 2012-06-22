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

#ifndef NUCLEX_SUPPORT_CONFIG_H
#define NUCLEX_SUPPORT_CONFIG_H

// --------------------------------------------------------------------------------------------- //

/// \mainpage
/// <para>
///   The Nuclex.Storage.Native library completely abstracts the file system from your
///   application and lets you access files and folders through a unified interface that
///   stays the same regardless of what system your game is running on. It follows a very
///   simple interface, allows for multi-threaded access to files and can seamlessly access
///   compressed files inside .zip archive as if they were plain files.
/// </para>
/// <list>
///   <item>
///     <description>
///       You are completely abstracted from details such as the target system's file API and
///       path separators. This ensures you don't have to rewrite file system interop each
///       time you port an application to a different platform.<br /><br />
///     </description>
///   </item>
///   <item>
///     <description>
///       Using the BinaryReader and BinaryWriter classes will also isolate you from issues
///       arising from endian issues, automatically flipping the bytes as required with
///       the smallest overhead possible on each platform.<br /><br />
///     </description>
///   </item>
///   <item>
///     <description>
///       Transparently access .zip archives (and any other archive for which you implement
///       a ContainerFileCodec) as if they were plain folders. No prior extraction or different
///       methods to use, a resource loader won't even notice whether the data comes from
///       a file in a .zip archive or a plain file. The built-in .zip decompressor supports
///       fast, multi-threaded random access to compressed data.<br /><br />
///     </description>
///   </item>
///   <item>
///     <description>
///       File access is simplified by getting rid of Open() and Close() calls - simply read
///       from a file and write to a file by calling the ReadAt() or WriteAt() methods.
///       If the implementation requires it, file handles will be created as needed.
///       Games normally read from packaged files, where opening a packaged file is a no-op
///       and would only cause more management work for multi-threaded accesses.<br /><br />
///     </description>
///   </item>
///   <item>
///     <description>
///       Files can be read from multiple threads without any synchronization issues. Because
///       there is no file-global cursor dictating where the next read or write will occur,
///       any number of threads can safely access a file simultaneously.<br /><br />
///     </description>
///   </item>
///   <item>
///     <description>
///       The SerializationManager enables you to write separate serializers, keeping
///       your classes free from serialization code (if you wish) and enabling you to load
///       and save arbitrary classes even if they were designed without knowledge of
///       the load/save system.<br /><br />
///     </description>
///   </item>
/// </list>
/// <para>
///   From the file manager, you can access the file system starting at 5 entry paths.
///   These are: <strong>Install</strong> - the directory your game has been installed
///   in. Always read-only. <strong>SaveGame</strong> - a directory where you can store saved
///   games, mapping either to the operating system's "Saved Games" directory, if available,
///   or using whatever is common on the target system. <strong>Personal</strong> - personal
///   data of the player, stored in the local user profile or in the cloud.
///   <strong>Local</strong> - for data that is only relevant to the machine the application
///   is running on, such as the selected hardware devices. <strong>Temporary</strong> -
///   a local for you to cache things like compiled shaders or scripting language byte-codes.
/// </para>

// --------------------------------------------------------------------------------------------- //

// Platform recognition
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  #define NUCLEX_SUPPORT_WINRT 1
#elif defined(WIN32) || defined(_WIN32)
  #define NUCLEX_SUPPORT_WIN32 1
#else
  #define NUCLEX_SUPPORT_LINUX 1
#endif

// --------------------------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (Nuclex.Storage.Native library). The NUCLEX_SUPPORT_SOURCE symbol is defined by
// all source files of the library, so you don't have to worry about a thing.
#if defined(_MSC_VER)

  #if defined(NUCLEX_SUPPORT_STATICLIB)
    #define NUCLEX_SUPPORT_API
  #else
    #if defined(NUCLEX_SUPPORT_SOURCE)
      // If we are building the DLL, export the symbols tagged like this
      #define NUCLEX_SUPPORT_API __declspec(dllexport)
    #else
      // If we are consuming the DLL, import the symbols tagged like this
      #define NUCLEX_SUPPORT_API __declspec(dllimport)
    #endif
  #endif

#elif defined(__GNUC__)

  #if defined(NUCLEX_SUPPORT_STATICLIB)
    #define NUCLEX_SUPPORT_API
  #else
    #if defined(NUCLEX_SUPPORT_SOURCE)
      #define NUCLEX_SUPPORT_API __attribute__ ((visibility ("default")))
      // Make hidden the default visibility so only tagged symbols are exported
      // This only applies when compiling Nuclex.Storage.Native, so we don't infect
      // any code merely using the library with this line.
      //#pragma GCC visibility push(hidden)
    #else
      // If you use -fvisibility=hidden or the same pragma as above anywhere in GCC,
      // exception handling and RTTI would break because GCC would immediately forget
      // all type infos encountered without this. See http://gcc.gnu.org/wiki/Visibility
      #define NUCLEX_SUPPORT_API __attribute__ ((visibility ("default")))
    #endif
  #endif

#endif

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_SUPPORT_CONFIG_H
