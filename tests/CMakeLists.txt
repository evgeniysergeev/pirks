
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

set(COMMON_TEST_SOURCES
    "common-tests/CircularBufferTest.cpp"
)

add_executable(pirks-tests ${COMMON_TEST_SOURCES})

target_link_libraries(pirks-tests
    common
    default_compiler_flags
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(pirks-tests)
