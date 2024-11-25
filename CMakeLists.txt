cmake_minimum_required(VERSION 3.15)

# set the project name
project(Pirks VERSION 0.0.1.0)

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

# generate config header file
configure_file(src/version.h.in version.h)

set(PIRKS_SRC
    "src/main.cpp"
)

# add the executable
add_executable(
    pirks
    ${PIRKS_SRC}
)

# definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(pirks PRIVATE DEBUG=1)
else()
    target_compile_definitions(pirks PRIVATE RELEASE=1)
endif()

# configure include directories
target_include_directories(pirks PUBLIC
    ${PROJECT_BINARY_DIR}
    "src"
)

# add subdirectories
add_subdirectory(src/common)
add_subdirectory(src/server)

# link libraries
target_link_libraries(pirks PUBLIC
    common
    server
    default_compiler_flags
)
