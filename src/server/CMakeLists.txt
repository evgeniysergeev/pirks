# Server

set(SERVER_SOURCES
    "main.cpp"
    "server_config.h"
    "server_config.cpp"
)

add_executable(pirks-server ${SERVER_SOURCES})

# add to include directories
# ${PROJECT_BINARY_DIR} needed for include version.h was already added in common
target_include_directories(pirks-server INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(pirks-server PUBLIC
    capture_audio
    default_compiler_flags
    ${EXTERNAL_LIBRARIES}
)
