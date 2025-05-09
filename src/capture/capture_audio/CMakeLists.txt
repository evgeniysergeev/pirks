set(SOURCES
    "IMicrophone.h"
    "IMicrophoneFactory.h"
    "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.h"
    "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.c"
)

if(PLATFORM STREQUAL "Windows")
    set(PLATFORM_SOURCES
        "windows/WinMicrophoneFactory.h"
        "windows/WinMicrophoneFactory.cpp"
    )
endif()

if(PLATFORM STREQUAL "MACOS")
    set(PLATFORM_SOURCES
        "macos/AVAudio.h"
        "macos/AVAudio.m"
        "macos/MacMicrophone.h"
        "macos/MacMicrophone.mm"
        "macos/MacMicrophoneFactory.h"
        "macos/MacMicrophoneFactory.mm"
    )
endif()

add_library(capture_audio STATIC
    ${SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(capture_audio PUBLIC
    "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer"
)

target_include_directories(capture_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(capture_audio PUBLIC 
    common
    default_compiler_flags
)
