
# Based on tutorial from https://google.github.io/googletest/quickstart-cmake.html

enable_testing()

add_subdirectory(common-test)
add_subdirectory(common-debug-test)

if(TEST_MICROPHONE)
    add_subdirectory(microphone-test)
endif()
