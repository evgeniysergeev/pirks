set(COMMON_SRC
    "str_utils.h"
    "str_utils.cpp"
)

add_library(common STATIC ${COMMON_SRC})

# add to include directories
target_include_directories(pirks
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requrements from interface library with compiler flags
target_link_libraries(common PUBLIC default_compiler_flags)
