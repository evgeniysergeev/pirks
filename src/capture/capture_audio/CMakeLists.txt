set(SOURCES
    "IAudioInput.h"
    "IAudioInputFactory.h"
    "CaptureResult.h"
)

if(PLATFORM STREQUAL "WINDOWS")
    set(PLATFORM_SOURCES
        "windows/WinAudioInputFactory.h"
        "windows/WinAudioInputFactory.cpp"
    )
endif()

if(PLATFORM STREQUAL "MACOS")
    set(PLATFORM_SOURCES
        "macos/CaptureDevice.h"
        "macos/CaptureDevice.m"
        "macos/MacAudioInput.h"
        "macos/MacAudioInput.mm"
        "macos/MacAudioInputFactory.h"
        "macos/MacAudioInputFactory.mm"
        "macos/TPCircularBufferHelper.h"
        "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.h"
        "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.c"
    )
endif()

add_library(capture_audio STATIC
    ${SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(capture_audio PUBLIC
    "${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer"
)

if(PLATFORM STREQUAL "WINDOWS")
    target_include_directories(capture_audio INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/windows
    )
endif()

if(PLATFORM STREQUAL "MACOS")
    target_include_directories(capture_audio INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/macos
    )
endif()

target_include_directories(capture_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# use requirements from interface library with compiler flags
target_link_libraries(capture_audio PUBLIC 
    common
    default_compiler_flags
)
