# Add the dependency on GoogleTest which is downloaded from GitHub
# Based on tutorial at this url: https://google.github.io/googletest/quickstart-cmake.html

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24 and greater:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.17.x.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
