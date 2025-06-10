
# Based on common-test

set(TARGET_NAME common-debug-test)

set(SOURCES
    memory_utils_test.cpp
)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME}
    common
    default_compiler_flags
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
