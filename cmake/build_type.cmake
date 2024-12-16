# Use Release if none specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(default_compiler_flags INTERFACE
        DEBUG=1
    )
else()
    # if NDEBUG is defined, all asserts is disabled
    target_compile_definitions(default_compiler_flags INTERFACE
        RELEASE=1
        NDEBUG
    )
endif()
