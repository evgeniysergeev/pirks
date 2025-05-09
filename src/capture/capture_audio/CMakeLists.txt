set(CAPTURE_AUDIO_SRC
    "IMicrophone.h"
    "IMicrophoneFactory.h"
    "windows/WindowsMicrophoneFactory.h"
    "windows/WindowsMicrophoneFactory.cpp"
)

add_library(capture_audio STATIC ${CAPTURE_AUDIO_SRC})

target_include_directories(capture_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(capture_audio PUBLIC 
    common
    default_compiler_flags
)
