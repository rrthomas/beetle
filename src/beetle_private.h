// Header for C Beetle containing all the private implementation-specific
// APIs that are shared between modules.
//
// (c) Reuben Thomas 1994-2018

#ifndef BEETLE_BEETLE_PRIVATE
#define BEETLE_BEETLE_PRIVATE


#include "config.h"

// Memory access

// Address checking
#define CHECK_ADDRESS(a, cond, code, label)     \
    if (!(cond)) {                              \
        NOT_ADDRESS = a;                        \
        exception = code;                       \
        goto label;                             \
    }

#define CHECK_ALIGNED(a)                                \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, badadr)


#endif
