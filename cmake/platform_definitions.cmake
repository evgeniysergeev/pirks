# Add platform definition
if(APPLE)
    set(PLATFORM "MACOS")
    target_compile_definitions(default_compiler_flags INTERFACE
        UNIX
        MACOS
        PROJECT_PLATFORM="macOS"
    )
elseif(UNIX AND NOT APPLE)
    set(PLATFORM "LINUX")
    target_compile_definitions(default_compiler_flags INTERFACE
        UNIX
        LINUX
        PROJECT_PLATFORM="Linux"
    )
elseif(WIN32)
    set(PLATFORM "WINDOWS")
    target_compile_definitions(default_compiler_flags INTERFACE
        WINDOWS
        WIN32_LEAN_AND_MEAN
        PROJECT_PLATFORM="Windows"
    )
else()
    message(FATAL_ERROR "Unknown build platform.")
endif()
