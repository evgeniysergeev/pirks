set(SERVER_SRC
    "server_config.h"
    "server_config.cpp"
)

add_library(server STATIC ${SERVER_SRC})

# add to include directories
target_include_directories(pirks
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requrements from interface library with compiler flags
target_link_libraries(common PUBLIC default_compiler_flags)
