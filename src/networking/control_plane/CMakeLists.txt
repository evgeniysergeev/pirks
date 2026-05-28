set(SOURCES
    ControlPlaneConfig.h
    ControlPlaneMessage.h
    ControlPlaneMessage.cpp
    ControlPlaneServer.h
    ControlPlaneServer.cpp
)

add_library(control_plane STATIC
    ${SOURCES}
)

target_include_directories(control_plane PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(control_plane INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(OpenSSL REQUIRED)

target_link_libraries(control_plane PUBLIC
    common
    default_compiler_flags
    Boost::asio
    boost::beast
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_libraries(control_plane PUBLIC
        ws2_32
        mswsock
        crypt32
        bcrypt
    )
endif()
