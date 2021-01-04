#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexSupportNative
	VERSION 1.0.0
	DESCRIPTION "Utility functions and basic interface definitions"
)

include("../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)

#set(GoogleTest_DIR "../ThirdParty/gtest")
#find_package(GoogleTest REQUIRED CONFIG)

find_package(Threads REQUIRED)

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

if(WIN32)
	set_target_properties(NuclexSupportNative PROPERTIES OUTPUT_NAME "Nuclex.Support.Native")
endif()

# -------------------------------------------------------------------------------------------------

add_executable(NuclexSupportNativeTests)

target_compile_definitions(
	NuclexSupportNativeTests
	PRIVATE NUCLEX_SUPPORT_EXECUTABLE
)

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

target_link_libraries(
	NuclexSupportNativeTests
	PRIVATE GoogleTest
	PRIVATE GoogleTestMain
	PRIVATE Threads::Threads
)

if(WIN32)
	set_target_properties(
		NuclexSupportNativeTests PROPERTIES OUTPUT_NAME "Nuclex.Support.Native.Tests"
	)
endif()

# -------------------------------------------------------------------------------------------------

install(
	TARGETS NuclexSupportNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install(
	TARGETS NuclexSupportNativeTests
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------
