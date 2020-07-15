// Auxiliary public functions.
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef beetle_BEETLE_AUX
#define beetle_BEETLE_AUX


#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Stacks size
#define beetle_DEFAULT_STACK_SIZE   16384

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
beetle_CELL beetle_reverse_cell(beetle_CELL value);
int beetle_reverse(beetle_UCELL start, beetle_UCELL length);

#define beetle_STACK_DIRECTION -1
#define beetle__LOAD_CELL(a, temp)                                             \
    ((exception = exception ? exception : beetle_load_cell((a), &temp)), temp)
#define beetle_LOAD_CELL(a) beetle__LOAD_CELL(a, temp)
#define beetle_STORE_CELL(a, v)                                                \
    (exception = exception ? exception : beetle_store_cell((a), (v)))
#define beetle_LOAD_BYTE(a)                                                    \
    ((exception = exception ? exception : beetle_load_byte((a), &byte)), byte)
#define beetle_STORE_BYTE(a, v)                                                \
    (exception = exception ? exception : beetle_store_byte((a), (v)))
#define beetle_PUSH(v)                                 \
    (beetle_SP += beetle_CELL_W * beetle_STACK_DIRECTION, beetle_STORE_CELL(beetle_SP, (v)))
#define beetle_POP                                     \
    (beetle_SP -= beetle_CELL_W * beetle_STACK_DIRECTION, beetle_LOAD_CELL(beetle_SP + beetle_CELL_W * beetle_STACK_DIRECTION))
#define beetle_PUSH_DOUBLE(ud)                         \
    beetle_PUSH((beetle_UCELL)(ud & beetle_CELL_MASK));              \
    beetle_PUSH((beetle_UCELL)((ud >> beetle_CELL_BIT) & beetle_CELL_MASK))
#define beetle_POP_DOUBLE                              \
    (beetle_SP -= 2 * beetle_CELL_W * beetle_STACK_DIRECTION, (beetle_UCELL)beetle_LOAD_CELL(beetle_SP + beetle_CELL_W * beetle_STACK_DIRECTION), temp | \
     ((beetle_DUCELL)(beetle_UCELL)beetle__LOAD_CELL(beetle_SP + 2 * beetle_CELL_W * beetle_STACK_DIRECTION, temp2) << beetle_CELL_BIT))
#define beetle_PUSH_RETURN(v)                          \
    (beetle_RP += beetle_CELL_W * beetle_STACK_DIRECTION, beetle_STORE_CELL(beetle_RP, (v)))
#define beetle_POP_RETURN                              \
    (beetle_RP -= beetle_CELL_W * beetle_STACK_DIRECTION, beetle_LOAD_CELL(beetle_RP + beetle_CELL_W * beetle_STACK_DIRECTION))
#define beetle_STACK_UNDERFLOW(ptr, base)              \
    (ptr - base == 0 ? false : (beetle_STACK_DIRECTION > 0 ? ptr < base : ptr > base))

uint8_t *native_address_of_range(beetle_UCELL start, beetle_UCELL length);

// Align a VM address
#define beetle_ALIGN(a) ((a + beetle_CELL_W - 1) & (-beetle_CELL_W))

// Check whether a VM address is aligned
#define beetle_IS_ALIGNED(a)     (((a) & (beetle_CELL_W - 1)) == 0)

// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#define beetle_ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (beetle_CELL_BIT - p)))


#endif
