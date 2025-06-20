set(SOURCES
)

add_library(tcp_net STATIC
    ${SOURCES}
)

target_include_directories(tcp_net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(tcp_net PUBLIC 
    common
    default_compiler_flags
)
