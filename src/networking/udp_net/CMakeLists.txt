set(SOURCES
)

add_library(udp_net STATIC
    ${SOURCES}
)

target_include_directories(udp_net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(udp_net PUBLIC 
    common
    default_compiler_flags
)
