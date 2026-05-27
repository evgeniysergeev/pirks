# Setup Beast v2 (header-only) via FetchContent
# Based on: https://www.boost.org/doc/libs/master/libs/beast/doc/html/index.html

if(TARGET boost::beast)
    message(STATUS "Beast is already available.")
else()
    # Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24 and greater:
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()

    include(FetchContent)

    FetchContent_Declare(
        beast_v2
        URL https://github.com/boostorg/beast/archive/refs/heads/develop.zip
    )

    # Beast v2 is header-only; use MakeAvailable but override the target creation
    FetchContent_MakeAvailable(beast_v2)

    # The boost::beast target may not be created by Boost.Build for develop branch.
    # Ensure it exists as an INTERFACE IMPORTED library with correct include path.
    if(NOT TARGET boost::beast)
        add_library(boost::beast INTERFACE IMPORTED)
        target_include_directories(boost::beast SYSTEM INTERFACE
            ${beast_v2_SOURCE_DIR}/include
        )
    endif()
endif()
