cmake_minimum_required(VERSION 3.15)

# set the project name
project(Pirks VERSION 1.0.0.0)

set(PROJECT_LICENSE "AGPL-3")

set(PROJECT_LONG_DESCRIPTION "Pirks is a video streaming applcation")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# determine which compiler is currently using to build
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

# create interface library for compiler flags
add_library(default_compiler_flags INTERFACE)

# specify the C++ standard
target_compile_features(default_compiler_flags INTERFACE cxx_std_20)

# set default compiler warning flags
target_compile_options(default_compiler_flags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

include(cmake/platform_definitions.cmake)
include(cmake/debug_definitions.cmake)

# generate config header file
configure_file(src/version.h.in version.h)

# add common static library subdirectories
add_subdirectory(src/common)

# add server subdirectory
add_subdirectory(src/server)
