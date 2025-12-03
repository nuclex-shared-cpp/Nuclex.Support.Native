#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.15)

# -------------------------------------------------------------------------------------------------

project(
  NuclexSupportNative
  VERSION 1.0.0
  DESCRIPTION "Utility functions and basic interface definitions"
)

option(
  BUILD_DOCS
  "Whether to generate documentation via Doxygen"
  OFF
)
option(
  BUILD_UNIT_TESTS
  "Whether to build the unit test executable. This will require an extra \
  compilation of the entire source tree as well as the GoogleTest library."
  OFF
)
option(
  BUILD_BENCHMARK
  "Whether to build the benchmark executable. This will require an extra \
  compilation of the entire source tree as well as the Celero library."
  OFF
)
option(
  BENCHMARK_THIRD_PARTY_LIBRARIES
  "Whether to include third-party libraries in the benchmarks. If you enable \
  this option, you need to manually place the extra libraries into your \
  source tree (easiest done by just enabling and checking the missing files \
  in the compiler output)."
  OFF
)

# -------------------------------------------------------------------------------------------------

# This sets a bunch of compile flags and defined ${NUCLEX_COMPILER_TAG} to
# say something like linux-gcc-13.2-amd64-debug. You should have this directory
# if you do a full clone of a project that is using this third-party library build.
include("../build-system/cmake/cplusplus.cmake")

# The Unix build pipeline doesn't automatically include threading, so search for
# the pthreads library in order to link against it later on.
#   https://en.wikipedia.org/wiki/Pthreads
find_package(Threads REQUIRED)

# Add MoodyCamelQueues as a sub-project, it provides fast, lock-free concurrent queues
if(NOT (TARGET MoodyCamelQueue))
  add_subdirectory(
    ${PROJECT_SOURCE_DIR}/../third-party/nuclex-moodycamelqueues
    ${CMAKE_BINARY_DIR}/nuclex-moodycamelqueues
  )
endif()

# Add DragonBox as a sub-project, it's the fastest float printer out there
if(NOT (TARGET DragonBox))
  add_subdirectory(
    ${PROJECT_SOURCE_DIR}/../third-party/nuclex-dragonbox
    ${CMAKE_BINARY_DIR}/nuclex-dragonbox
  )
endif()

# Add FastFloat as a sub-project, it's the fastest float parser out there
if(NOT (TARGET FastFloat))
  add_subdirectory(
    ${PROJECT_SOURCE_DIR}/../third-party/nuclex-fastfloat
    ${CMAKE_BINARY_DIR}/nuclex-fastfloat
  )
endif()

message(STATUS "Enabled options for Nuclex.Support.Native:")
message(STATUS "  ⚫ Build core library")

if(BUILD_UNIT_TESTS)
  message(STATUS "  ⚫ Build unit tests")

  # Add GoogleTest as a sub-project so we can link our unit test executable
  if(NOT (TARGET GoogleTest))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-googletest
      ${CMAKE_BINARY_DIR}/nuclex-googletest
    )
  endif()
endif()

if(BUILD_BENCHMARK)
  message(STATUS "  ⚫ Build benchmark")
  if(BENCHMARK_THIRD_PARTY_LIBRARIES)
    message(STATUS "  ⚫ Include third-party libraries in benchmark")
  endif()

  # Add Celero as a sub-project so we can link our benchmark executable
  if(NOT (TARGET Celero))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-celero
      ${CMAKE_BINARY_DIR}/nuclex-celero
    )
  endif()
endif()

# Use CMake's own package for locating Doxygen on the system
if(BUILD_DOCS)
  find_package(Doxygen)
endif()

# -------------------------------------------------------------------------------------------------

# Project structure
#
#   ProjectName/
#     Source/                   All source files, using deeper directories as needed
#     Include/ProjectName/      All public headers, using deeper directories as needed
#     Tests/                    All unit tests, using deeper directories as needed
#     Benchmarks/               All benchmark files, using deeper directories as needed
#
# From CMake documentation:
#   |  Note: We do not recommend using GLOB to collect a list of
#   |  source files from your source tree. If no CMakeLists.txt file
#   |  changes when a source is added or removed then the generated
#   |  build system cannot know when to ask CMake to regenerate.
#
# As so very often, CMake becomes a hurdle rather than helping.
# I'm not going to manually maintain a list of source files. Rebuilds
# where files are added, removed or renamed need to be from scratch.
#
file(
  GLOB_RECURSE sourceFiles
  CONFIGURE_DEPENDS
  "Source/*.cpp"
  "Source/*.c"
)
file(
  GLOB_RECURSE headerFiles
  CONFIGURE_DEPENDS
  "Include/Nuclex/Support/*.h"
)
file(
  GLOB_RECURSE unittestFiles
  CONFIGURE_DEPENDS
  "Tests/*.cpp"
)
file(
  GLOB_RECURSE benchmarkFiles
  CONFIGURE_DEPENDS
  "Benchmarks/*.cpp"
)

# -------------------------------------------------------------------------------------------------

# Shared library that can be linked to other projects
add_library(NuclexSupportNative SHARED)

# Enable compiler warnings only if this library is compiled on its own.
# If it's used as a sub-project, the including project's developers aren't
# interested in seeing warnings from a project they're not maintaining.
if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexSupportNative")
  enable_target_compiler_warnings(NuclexSupportNative)
else()
  disable_target_compiler_warnings(NuclexSupportNative)
endif()

# Add directory with public headers to include path
target_include_directories(
  NuclexSupportNative
  PUBLIC "Include"
)

# Add public headers and sources to compilation list
# (headers, too, in case CMake is used to generate an IDE project)
target_sources(
  NuclexSupportNative
  PUBLIC ${headerFiles}
  PRIVATE ${sourceFiles}
)

# Link against PThreads
target_link_libraries(
  NuclexSupportNative
  PRIVATE Threads::Threads
  PRIVATE MoodyCamelQueues::Static
  PRIVATE DragonBox::Static
  PRIVATE FastFloat::Static
)

# On Windows, we want the shared library to be named Nuclex.Support.Native.dll
if(WIN32)
  target_link_libraries(
    NuclexSupportNative
    PRIVATE synchronization.lib
  )
  set_target_properties(
    NuclexSupportNative
    PROPERTIES OUTPUT_NAME "Nuclex.Support.Native"
  )
endif()

# -------------------------------------------------------------------------------------------------

if(BUILD_UNIT_TESTS)

  # Executable that runs the unit tests (main() supplied by GoogleTest)
  add_executable(NuclexSupportNativeTests)

  # Enable compiler warnings only if this library is compiles on its own.
  # If it's used as a sub-project, the including project's developers aren't
  # interested in seeing warnings from a project they're not maintaining.
  if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexSupportNative")
    enable_target_compiler_warnings(NuclexSupportNativeTests)
  else()
    disable_target_compiler_warnings(NuclexSupportNativeTests)
  endif()

  # Let the code know it's not being compiled into a shared library
  # (this disables visibility/exports, thus allowing the compiler detect
  # additional unused code and warn about it)
  target_compile_definitions(
    NuclexSupportNativeTests
    PRIVATE NUCLEX_SUPPORT_EXECUTABLE
  )

  # Add directory with public headers to include path
  target_include_directories(
    NuclexSupportNativeTests
    PUBLIC "Include"
  )

  # Add public headers and sources (normal + unit tests) to compilation list
  # (headers, too, in case CMake is used to generate an IDE project)
  target_sources(
    NuclexSupportNativeTests
    PRIVATE ${headerFiles}
    PRIVATE ${sourceFiles}
    PRIVATE ${unittestFiles}
  )

  # Link GoogleTest and the main() function supplied by GoogleTest
  # Also link against PThreads
  target_link_libraries(
    NuclexSupportNativeTests
    PRIVATE GoogleTest::Static
    PRIVATE GoogleTest::Main
    PRIVATE Threads::Threads
    PRIVATE MoodyCamelQueues::Static
    PRIVATE DragonBox::Static
    PRIVATE FastFloat::Static
  )

  # On Windows, we want the executable to be named Nuclex.Support.Native.Tests.exe
  if(WIN32)
    target_link_libraries(
      NuclexSupportNativeTests
      PRIVATE synchronization.lib
    )
    set_target_properties(
      NuclexSupportNativeTests
      PROPERTIES OUTPUT_NAME "Nuclex.Support.Native.Tests"
    )
  endif()

endif() # if BUILD_UNIT_TESTS enabled

# -------------------------------------------------------------------------------------------------

if(BUILD_BENCHMARK)

  # Executable that runs the benchmark (main() supplied by Celero)
  add_executable(NuclexSupportNativeBenchmark)

  # Enable compiler warnings only if this library is compiles on its own.
  # If it's used as a sub-project, the including project's developers aren't
  # interested in seeing warnings from a project they're not maintaining.
  if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexSupportNative")
    enable_target_compiler_warnings(NuclexSupportNativeBenchmark)
  else()
    disable_target_compiler_warnings(NuclexSupportNativeBenchmark)
  endif()

  # Let the code know it's not being compiled into a shared library
  # (this disables visibility/exports, thus allowing the compiler detect
  # additional unused code and warn about it)
  target_compile_definitions(
    NuclexSupportNativeBenchmark
    PRIVATE NUCLEX_SUPPORT_EXECUTABLE
  )

  if(BENCHMARK_THIRD_PARTY_LIBRARIES)
    target_compile_definitions(
      NuclexSupportNativeBenchmark
      PRIVATE HAVE_BOOST_SIGNALS
      PRIVATE HAVE_LSIGNAL
      PRIVATE HAVE_NANO_SIGNALS
      PRIVATE HAVE_JEAIII_ITOA
    )
    if(NOT WIN32)
      target_compile_definitions(
        NuclexSupportNativeBenchmark
        PRIVATE HAVE_AMDN_ITOA
      )
    endif()
  endif()

  # Add directory with public headers to include path
  target_include_directories(
    NuclexSupportNativeBenchmark
    PUBLIC "Include"
  )

  # Add public headers and sources (normal + benchmark) to compilation list
  # (headers, too, in case CMake is used to generate an IDE project)
  target_sources(
    NuclexSupportNativeBenchmark
    PRIVATE ${headerFiles}
    PRIVATE ${sourceFiles}
    PRIVATE ${benchmarkFiles}
  )

  # Link GoogleTest and the main() function supplied by GoogleTest
  # Also link against PThreads
  target_link_libraries(
    NuclexSupportNativeBenchmark
    PRIVATE Celero::Static
    PRIVATE Threads::Threads
    PRIVATE MoodyCamelQueues::Static
    PRIVATE DragonBox::Static
    PRIVATE FastFloat::Static
  )

  # On Windows, we want the executable to be named Nuclex.Support.Native.Benchmark.exe
  if(WIN32)
    target_link_libraries(
      NuclexSupportNativeBenchmark
      PRIVATE synchronization.lib
    )
    set_target_properties(
      NuclexSupportNativeBenchmark
      PROPERTIES OUTPUT_NAME "Nuclex.Support.Native.Benchmark"
    )
  endif()

endif() # if BUILD_BENCHMARK enabled

# -------------------------------------------------------------------------------------------------

# Install the shared library into a subdirectory of this CMakeLists.txt file
# under ./bin/linux-gcc9.3-amd64-debug/ (the second-level directory is called
# "compiler tag" and dynamically formed -- it ensures that when linking
# a pre-compiled shared library, the correct library is used).
install(
  TARGETS NuclexSupportNative
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Install .pdb files on Windows platforms.
#
# CMake forgets installing .pdb files for debug builds. These are needed by
# Microsoft compilers when debugging to associate machine code locations to
# source files and line numbers.
install_debug_symbols(NuclexSupportNative)

# Install unit tests in same location as shared library.
if(BUILD_UNIT_TESTS)
  install(
    TARGETS NuclexSupportNativeTests
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  )

  # Install .pdb files on Windows platforms for the unit tests, too.
  install_debug_symbols(NuclexSupportNativeTests)
endif()

# Install benchmarks in same location as shared library.
if(BUILD_BENCHMARK)
  install(
    TARGETS NuclexSupportNativeBenchmark
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  )

  # Install .pdb files on Windows platforms for the unit tests, too.
  install_debug_symbols(NuclexSupportNativeBenchmark)
endif()

# Maintain a symlink to the most recent build directory in the bin directory.
# This is useful to simplify IDE configurations (i.e. path to the executable
# that should be debugged, etc.)
install(
  CODE
  "execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
    ${PROJECT_SOURCE_DIR}/bin/last-build
  )"
)

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
  endif()

  add_custom_target(
    NuclexSupportNativeDocs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Support.Native.doxygen.cfg"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )

endif()
