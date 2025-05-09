set(SOURCES
    "IMicrophone.h"
    "IMicrophoneFactory.h"
)

if(PLATFORM STREQUAL "Windows")
    set(PLATFORM_SOURCES
        "windows/WindowsMicrophoneFactory.h"
        "windows/WindowsMicrophoneFactory.cpp"
    )
endif()

add_library(capture_audio STATIC
    ${SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(capture_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(capture_audio PUBLIC 
    common
    default_compiler_flags
)
