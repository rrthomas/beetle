// Header for C Beetle containing auxiliary public functions.
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef BEETLE_BEETLE_AUX
#define BEETLE_BEETLE_AUX


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Stacks location and size
#define DATA_STACK_SEGMENT   0xfe000000
#define RETURN_STACK_SEGMENT 0xff000000
#define DEFAULT_STACK_SIZE   16384
#define MAX_STACK_SIZE       67108864

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
CELL reverse_cell(CELL value);
int reverse(UCELL start, UCELL length);

#define STACK_DIRECTION 1
#define _LOAD_CELL(a, temp)                                             \
    ((exception = exception ? exception : load_cell((a), &temp)), temp)
#define LOAD_CELL(a) _LOAD_CELL(a, temp)
#define STORE_CELL(a, v)                                                \
    (exception = exception ? exception : store_cell((a), (v)))
#define LOAD_BYTE(a)                                                    \
    ((exception = exception ? exception : load_byte((a), &byte)), byte)
#define STORE_BYTE(a, v)                                                \
    (exception = exception ? exception : store_byte((a), (v)))
#define PUSH(v)                                 \
    (SP += CELL_W * STACK_DIRECTION, STORE_CELL(SP, (v)))
#define POP                                     \
    (SP -= CELL_W * STACK_DIRECTION, LOAD_CELL(SP + CELL_W * STACK_DIRECTION))
#define PUSH_DOUBLE(ud)                         \
    PUSH((UCELL)(ud & CELL_MASK));              \
    PUSH((UCELL)((ud >> CELL_BIT) & CELL_MASK))
#define POP_DOUBLE                              \
    (SP -= 2 * CELL_W * STACK_DIRECTION, (UCELL)LOAD_CELL(SP + CELL_W * STACK_DIRECTION), temp | \
     ((DUCELL)(UCELL)_LOAD_CELL(SP + 2 * CELL_W * STACK_DIRECTION, temp2) << CELL_BIT))
#define PUSH_RETURN(v)                          \
    (RP += CELL_W * STACK_DIRECTION, STORE_CELL(RP, (v)))
#define POP_RETURN                              \
    (RP -= CELL_W * STACK_DIRECTION, LOAD_CELL(RP + CELL_W * STACK_DIRECTION))
#define STACK_UNDERFLOW(ptr, base)              \
    (ptr - base == 0 ? false : (STACK_DIRECTION > 0 ? ptr < base : ptr > base))

uint8_t *native_address_range_in_one_area(UCELL start, UCELL length, bool writable);

// Align a Beetle address
#define ALIGNED(a) ((a + CELL_W - 1) & (-CELL_W))

// Check whether a Beetle address is aligned
#define IS_ALIGNED(a)     (((a) & (CELL_W - 1)) == 0)

// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (CELL_BIT - p)))

#define NEXT (exception = (EP += CELL_W, load_cell(EP - CELL_W, &A)))


#endif
