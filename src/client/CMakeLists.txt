# Client

set(TARGET_NAME pirks-client)

set(SOURCES
    main.cpp
    ClientConfig.h
    ClientConfig.cpp
    Client.h
    Client.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

# add to include directories
# ${PROJECT_BINARY_DIR} needed for include version.h was already added in common
target_include_directories(${TARGET_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(OpenSSL REQUIRED)

target_link_libraries(${TARGET_NAME} PUBLIC
    control_plane
    default_compiler_flags
)
