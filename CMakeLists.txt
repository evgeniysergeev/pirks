cmake_minimum_required(VERSION 3.15)

# set the project name
project(Pirks VERSION 1.0.0.0)

set(PROJECT_LICENSE "AGPL-3")

set(PROJECT_LONG_DESCRIPTION "Pirks is a video streaming application")

# Build and run tests
option(BUILD_TESTS "Build and run tests" ON)

include(cmake/default_compiler_flags.cmake)
include(cmake/platform_definitions.cmake)
include(cmake/build_type.cmake)
include(cmake/spdlog_setup.cmake)
include(cmake/cli11_setup.cmake)
include(cmake/deferral_setup.cmake)

if(PLATFORM STREQUAL "MACOS")
    include(cmake/macos.cmake)
endif()

# generate config header file
configure_file(src/version.h.in version.h)

# add common static library subdirectory
add_subdirectory(src/common)

# add capture static libraries subdirectory
add_subdirectory(src/capture)

# add server subdirectory
add_subdirectory(src/server)

# Tests
if(BUILD_TESTS)
    # Fine tune tests options
    include(cmake/tests_options.cmake)

    include("cmake/fetch_googletest.cmake")
    add_subdirectory(test)
endif()
