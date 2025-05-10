
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(TARGET_NAME pirks-tests)

set(SOURCES
    common-tests/CircularBufferTest.cpp
)

if(TEST_MICROPHONE)
    list(APPEND SOURCES
        microphone-tests/MicrophoneTest.cpp
    )
endif()

add_executable(pirks-tests ${SOURCES})

if(TEST_MICROPHONE)
    if(PLATFORM STREQUAL "MACOS")
        set(PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macos/info.plist")
        target_link_options(${TARGET_NAME} PRIVATE
            LINKER:-sectcreate,__TEXT,__info_plist,${PLIST_FILE}
        )
    endif()
endif()

target_link_libraries(${TARGET_NAME}
    capture_audio
    default_compiler_flags
    GTest::gtest_main
    ${EXTERNAL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})

# codesign --verbose=3 -s - --force --deep pirks-tests