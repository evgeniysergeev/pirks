# add DEBUG or Release definition
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(default_compiler_flags INTERFACE
        DEBUG=1
    )
else()
    target_compile_definitions(default_compiler_flags INTERFACE
        RELEASE=1
    )
endif()