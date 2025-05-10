
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(SOURCES
    "common-tests/CircularBufferTest.cpp"
    "microphone-tests/MicrophoneTest.cpp"
)

add_executable(pirks-tests ${SOURCES})

target_link_libraries(pirks-tests
    common
    capture_audio
    default_compiler_flags
    GTest::gtest_main
    ${EXTERNAL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(pirks-tests)
