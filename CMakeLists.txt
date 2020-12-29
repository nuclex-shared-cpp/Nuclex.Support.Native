#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexSupportNative
	VERSION 1.0.0
	DESCRIPTION "Utility functions and basic interface definitions"
)

include("../BuildSystem/cmake/cplusplus.cmake")

#variable_watch(sourceFiles)
#variable_watch(headerFiles)

add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)

# -------------------------------------------------------------------------------------------------

# "Note: We do not recommend using GLOB to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed then the generated
# build system cannot know when to ask CMake to regenerate."
#
# What are you f-ing smoking over there?
# Why would I even use this turd of a Makefile generator if I have to list my source files by hand?

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

# -------------------------------------------------------------------------------------------------

add_library(NuclexSupportNative SHARED)

target_include_directories(
	NuclexSupportNative
	PUBLIC "Include"
)

target_sources(
	NuclexSupportNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

install(
	TARGETS NuclexSupportNative
	LIBRARY
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

add_executable(NuclexSupportNativeTests)

target_include_directories(
	NuclexSupportNativeTests
	PUBLIC "Include"
)

target_sources(
	NuclexSupportNativeTests
	PRIVATE ${headerFiles}
	PRIVATE ${sourceFiles}
	PRIVATE ${unittestFiles}
)

find_package(Threads REQUIRED)

target_link_libraries(
	NuclexSupportNativeTests
	PRIVATE GoogleTest
	PRIVATE GoogleTestMain
	PRIVATE Threads::Threads
)

install(
	TARGETS NuclexSupportNativeTests
	RUNTIME
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------
