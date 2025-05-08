set(CAPTURE_AUDIO_SRC
    "IMicrophone.h"
    "str_utils2.cpp"
)

add_library(capture_audio STATIC ${CAPTURE_AUDIO_SRC})

# add to include directories
# ${PROJECT_BINARY_DIR} added for include version.h which generated here
target_include_directories(capture_audio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(capture_audio PUBLIC 
    common
    default_compiler_flags
)
