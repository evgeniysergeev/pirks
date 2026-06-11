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
        boost_headers
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.zip
    )

    if(POLICY CMP0169)
        cmake_policy(PUSH)
        cmake_policy(SET CMP0169 OLD)
    endif()

    FetchContent_GetProperties(boost_headers)
    if(NOT boost_headers_POPULATED)
        FetchContent_Populate(boost_headers)
    endif()

    if(POLICY CMP0169)
        cmake_policy(POP)
    endif()

    FetchContent_Declare(
        beast_v2
        URL https://github.com/boostorg/beast/archive/refs/heads/develop.zip
    )

    # Beast v2 is header-only; use MakeAvailable but override the target creation
    FetchContent_MakeAvailable(beast_v2)

    set(BOOST_HEADERS_SOURCE_DIR "${boost_headers_SOURCE_DIR}")
    if(EXISTS "${BOOST_HEADERS_SOURCE_DIR}/libs")
        file(GLOB BOOST_LIBRARY_INCLUDE_DIRS CONFIGURE_DEPENDS
            "${BOOST_HEADERS_SOURCE_DIR}/libs/*/include"
            "${BOOST_HEADERS_SOURCE_DIR}/libs/*/*/include"
        )
    else()
        message(FATAL_ERROR "Boost headers were not found at ${BOOST_HEADERS_SOURCE_DIR}")
    endif()

    if(NOT TARGET Boost::asio)
        add_library(Boost::asio INTERFACE IMPORTED)
        target_include_directories(Boost::asio SYSTEM INTERFACE
            ${BOOST_LIBRARY_INCLUDE_DIRS}
        )
    endif()

    # The boost::beast target may not be created by Boost.Build for develop branch.
    # Ensure it exists as an INTERFACE IMPORTED library with correct include path.
    if(NOT TARGET boost::beast)
        add_library(boost::beast INTERFACE IMPORTED)
        target_include_directories(boost::beast SYSTEM INTERFACE
            ${beast_v2_SOURCE_DIR}/include
            ${BOOST_LIBRARY_INCLUDE_DIRS}
        )
    endif()
endif()
