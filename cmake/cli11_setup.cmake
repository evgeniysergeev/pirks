# Setup CLI11 command line parameters parser
add_subdirectory(third-party/CLI11)

target_link_libraries(default_compiler_flags INTERFACE
    CLI11::CLI11
)
