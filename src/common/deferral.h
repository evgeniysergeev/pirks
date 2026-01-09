#pragma once

// deferral library uses __builtin_expect without check for MSVC or GCC or clang compiler
// To be able to compile using msvc compiler, need to make fake __builtin_expect
// TODO: remove this workaround in the future, when deferral library will fix this bug
#if defined(_MSC_VER)
#define __builtin_expect(value, likely_value) (value)
#endif

#include <deferral.hh>
