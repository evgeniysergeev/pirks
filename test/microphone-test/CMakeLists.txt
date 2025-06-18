
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(TARGET_NAME microphone-test)

set(SOURCES
    MicrophoneTest.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

# To Access Microphone in this test, Applicaiton need info.plist with 
# NSMicrophoneUsageDescription key set. 
# To be able to use info.plist in test, bundle is used instead of just
# simple console application.
if(PLATFORM STREQUAL "MACOS")
    set(PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macos/info.plist")

    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${TARGET_NAME}
        MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} # Use own plist template
        MACOSX_BUNDLE TRUE
    )
endif()

# Codesign not used for now
# TODO: Is it really need?
# codesign --verbose=3 --force --deep --sign - ${TARGET_NAME}
#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#    COMMAND codesign
#    ARGS --verbose=3 --force --deep --sign - ${TARGET_NAME}.app
#    COMMENT "Execute codesign to sign ${TARGET_NAME}"
# )

target_link_libraries(${TARGET_NAME}
    capture_audio
    default_compiler_flags
    GTest::gtest_main
    ${EXTERNAL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
