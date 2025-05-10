
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

set(TARGET_NAME common-test)

set(SOURCES
    CircularBufferTest.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME}
    common
    default_compiler_flags
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
