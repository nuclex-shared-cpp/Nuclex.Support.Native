Nuclex.Support.Native Dependencies
===================================


To Compile the Library
----------------------

This library is intended to be placed into a source tree using submodules to replicate
the following directory layout:

    root/
        Nuclex.Support.Native/          <-- you are here
            CMakeLists.txt

        build-system/                   <-- Git: nuclex-shared/build-system
            cmake/
                cplusplus.cmake

        third-party/
            nuclex-moodycamelqueues/    <-- Git: nuclex-builds/nuclex-moodycamelqueues
                CMakeLists.txt
            nuclex-googletest/          <-- Git: nuclex-builds/nuclex-googletest
                CMakeLists.txt
            nuclex-celero/              <-- Git: nuclex-builds/nuclex-ceelero
                CMakeLists.txt

If that's a bit overwhelming, try cloning (with `--recurse-submodules`) my "frame fixer"
or another application that uses this library to get that directory tree.

The dependencies of the code itself are very tame:

  * concurrentqueue by cameron314 (used internaly)
  * gtest (optional, if unit tests are built)
  * celero (optional, if benchmarks are built)


To Use this Library as a Binary
-------------------------------

Either use the included `CMakeLists.txt` (it still requires the `build-system` directory)
or, more directly:

  * Add `Nuclex.Support.Native/Include` to your include directory
  * Link `libNuclexSupportNative.so` (or `Nuclex.Support.Native.lib` on Windows)
  * Copy the `.so` file (or `.dll` file on Windows) to your output directory

