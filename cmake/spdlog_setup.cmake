# Setup spdlog. if packaged version of spdlog is found then use it
# If not found then use bundled spdlog
find_package(spdlog QUIET)
if(spdlog_FOUND)
    message(STATUS "Packaged version of spdlog will be used.")
else()
    message(STATUS "Bundled version of spdlog will be used.")

    add_subdirectory(third-party/spdlog)
endif()

target_link_libraries(default_compiler_flags INTERFACE spdlog::spdlog)
