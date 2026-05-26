
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(TARGET_NAME microphone-test)

set(SOURCES
    MicrophoneTest.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

# To access the microphone in this test, the application needs an info.plist
# with NSMicrophoneUsageDescription set. To make macOS privacy permissions
# stable across runs, give the test bundle a bundle identifier and ad-hoc sign
# it after each build.
if(PLATFORM STREQUAL "MACOS")
    set(PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macos/info.plist")

    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${TARGET_NAME}
        MACOSX_BUNDLE_GUI_IDENTIFIER org.pirks.microphone-test
        MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} # Use own plist template
        MACOSX_BUNDLE TRUE
    )

    find_program(CODESIGN_EXECUTABLE codesign)
    if(CODESIGN_EXECUTABLE)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} --force --deep --sign -
                    "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>"
            COMMENT "Ad-hoc signing ${TARGET_NAME}.app for macOS microphone privacy"
            VERBATIM
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
