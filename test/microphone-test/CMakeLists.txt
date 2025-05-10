
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(TARGET_NAME microphone-test)

set(SOURCES
    MicrophoneTest.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

# if(PLATFORM STREQUAL "MACOS")
#     set(PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macos/info.plist")
#     target_link_options(${TARGET_NAME} PRIVATE
#         LINKER:-sectcreate,__TEXT,__info_plist,${PLIST_FILE}
#     )
# endif()
#
# # codesign --verbose=3 --timestamp --options=runtime -s - --force --deep pirks-tests
# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#     COMMAND codesign
#     ARGS --verbose=3 --timestamp --options=runtime -s - --force --deep ${TARGET_NAME}
#     COMMENT "Execute codesign to sign ${TARGET_NAME}"
# )

target_link_libraries(${TARGET_NAME}
    capture_audio
    default_compiler_flags
    GTest::gtest_main
    ${EXTERNAL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
