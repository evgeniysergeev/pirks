#pragma once

/*

This is helper for include TPCircularBuffer.h with turned off asserts.
TPCircularBuffer.h has bug in assert which will produce warnings like this:

TPCircularBuffer/TPCircularBuffer.h:197:30: warning: comparison of integers of different signs: 'int' and 'uint32_t'
(aka 'unsigned int') [-Wsign-compare] [build]     assert(buffer->fillCount <= buffer->length); [build] ~~~~~~~~~~~~~~~~~
^  ~~~~~~~~~~~~~~

*/

#ifndef NDEBUG
#define NDEBUG
#define NEED_TO_UNDEF_NDEBUG
#endif

#include "TPCircularBuffer.h"

#ifdef NEED_TO_UNDEF_NDEBUG
#undef NDEBUG
#undef NEED_TO_UNDEF_NDEBUG
#endif
