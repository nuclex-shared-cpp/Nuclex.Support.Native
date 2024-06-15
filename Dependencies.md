Nuclex.Support.Native Dependencies
===================================

This library is intended to be placed into a project using submodules to replicate
the following directory layout:

    root/
        Nuclex.Support.Native/      <-- you are here
            CMakeLists.txt

        build-system/               <-- Git: nuclex-shared/build-system
            cmake/
                cplusplus.cmake

        third-party/
            nuclex-googletest/      <-- Git: nuclex-builds/nuclex-googletest
                CMakeLists.txt
            nuclex-celero/          <-- Git: nuclex-builds/nuclex-ceelero
                CMakeLists.txt

If that's a bit overwhelming, try cloning (with `--recurse-submodules` my "frame fixer"
or another application that uses this library).

The dependencies of the code itself are rather tame, or rather, zero:

    gtest (optional, if unit tests are built)
    celero (optional, if benchmarks are built)
