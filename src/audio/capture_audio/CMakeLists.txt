set(SOURCES
    AudioInputFactory.h
    CaptureResult.h
    IAudioInput.h
    IAudioInputFactory.h
)

if(PLATFORM STREQUAL "LINUX")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PULSEAUDIO REQUIRED IMPORTED_TARGET libpulse libpulse-simple)

    set(PLATFORM_SOURCES
        linux/LinuxAudioInputFactory.h
        linux/LinuxAudioInputFactory.cpp
        linux/PulseAudioInput.h
        linux/PulseAudioInput.cpp
    )
endif()

if(PLATFORM STREQUAL "WINDOWS")
    set(PLATFORM_SOURCES
        windows/AudioDeviceEnumerator.h
        windows/EndpointNotificationRegistration.h
        windows/MmcssTaskHandle.h
        windows/AudioFormats.h
        windows/AudioNotificationImpl.h
        windows/WasapiAudioClient.h
        windows/WasapiAudioInput.h
        windows/WasapiAudioInput.cpp
        windows/WinAudioInputFactory.h
        windows/WinAudioInputFactory.cpp
    )
endif()

if(PLATFORM STREQUAL "MACOS")
    set(MACOS_OBJECTIVE_C_HEADERS
        macos/CaptureDevice.h
        macos/MacAudioInput.h
        macos/MacAudioInputFactory.h
    )

    set(MACOS_OBJECTIVE_C_SOURCES
        macos/CaptureDevice.m
        macos/MacAudioInput.mm
        macos/MacAudioInputFactory.mm
    )

    set(PLATFORM_SOURCES
        ${MACOS_OBJECTIVE_C_HEADERS}
        ${MACOS_OBJECTIVE_C_SOURCES}
        ${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.h
        ${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer/TPCircularBuffer.c
    )

    # The macOS capture implementation uses manual retain/release/dealloc.
    # Keep these files on MRC unless the Objective-C ownership code is migrated.
    set_source_files_properties(${MACOS_OBJECTIVE_C_SOURCES}
        PROPERTIES COMPILE_FLAGS "-fno-objc-arc"
    )
endif()

add_library(capture_audio STATIC
    ${SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(capture_audio PUBLIC
    ${PROJECT_SOURCE_DIR}/third-party/TPCircularBuffer
)

if(PLATFORM STREQUAL "LINUX")
    target_include_directories(capture_audio INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/linux
    )
endif()

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

if(PLATFORM STREQUAL "WINDOWS")
    # Windows audio and MMCSS APIs:
    #  - avrt      for AvSetMmThreadCharacteristicsA / AvRevertMmThreadCharacteristics
    #  - mmdevapi  for MMDevice API (CLSID_MMDeviceEnumerator, IMMDeviceEnumerator, etc.)
    #  - uuid      for GUID/CLSID definitions
    #  - ksguid    for KS data format GUIDs (e.g. KSDATAFORMAT_SUBTYPE_PCM)
    target_link_libraries(capture_audio PUBLIC avrt mmdevapi uuid ksguid)
endif()

if(PLATFORM STREQUAL "LINUX")
    target_link_libraries(capture_audio PUBLIC PkgConfig::PULSEAUDIO)
endif()
