# Server

set(TARGET_NAME pirks-server)

set(SOURCES
    main.cpp
    ServerConfig.h
    ServerConfig.cpp
    Server.h
    Server.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

# add to include directories
# ${PROJECT_BINARY_DIR} needed for include version.h was already added in common
target_include_directories(${TARGET_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

# use requirements from interface library with compiler flags
target_link_libraries(${TARGET_NAME} PUBLIC
    capture_audio
    tcp_net
    default_compiler_flags
    ${EXTERNAL_LIBRARIES}
)
