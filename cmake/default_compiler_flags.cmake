# This flags and options will be applied to all targets

# determine which compiler is currently using to build
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

# create interface library for compiler flags
add_library(default_compiler_flags INTERFACE)

# specify the C++ standard
target_compile_features(default_compiler_flags INTERFACE cxx_std_23)

# set default compiler warning flags
target_compile_options(default_compiler_flags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Werror;-Wall;-Wextra;-Wpedantic;-Wcast-align;-Wcast-qual;-Wctor-dtor-privacy;-Wconversion;-Wenum-compare;-Wfloat-equal;-Wnon-virtual-dtor;-Woverloaded-virtual;-Wredundant-decls;-Wsign-conversion;-Wsign-promo;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/W4;/WX>>"
)
