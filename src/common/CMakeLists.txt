set(SOURCES
    "CircularBuffer.h"
    "str_utils.h"
    "str_utils.cpp"
)

add_library(common STATIC
    ${SOURCES}
)

# add to include directories
# ${PROJECT_BINARY_DIR} added for include version.h which generated here
target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(common PUBLIC
    default_compiler_flags
)
