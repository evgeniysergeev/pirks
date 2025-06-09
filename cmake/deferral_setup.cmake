# Setup deferral. if packaged version of deferral is found then use it
# If not found then use bundled deferral
find_package(deferral QUIET)
if(deferral_FOUND)
    message(STATUS "Packaged version of deferral will be used.")
else()
    message(STATUS "Bundled version of deferral will be used.")

    target_include_directories(default_compiler_flags INTERFACE
        "${PROJECT_SOURCE_DIR}/third-party/deferral/include"
    )
endif()
