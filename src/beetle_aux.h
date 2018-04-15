/* BEETLE_AUX.H

    (c) Reuben Thomas 1994-2018

    Header for C Beetle containing auxiliary public functions. These are undocumented,
    and subject to change, and not strictly necessary, but helpful.

*/


#ifndef BEETLE_BEETLE_AUX
#define BEETLE_BEETLE_AUX


#include "config.h"

#include <stdio.h>      /* for the FILE type */
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


#define DATA_STACK_SEGMENT   0xfe000000
#define RETURN_STACK_SEGMENT 0xff000000

/* Memory access */

/* Return value is 0 if OK, or exception code for invalid or unaligned
   address. */
CELL beetle_reverse_cell(CELL value);
int beetle_reverse(UCELL start, UCELL length);

#define _LOAD_CELL(a, temp)                                             \
    ((exception = exception ? exception : beetle_load_cell((a), &temp)), temp)
#define LOAD_CELL(a) _LOAD_CELL(a, temp)
#define STORE_CELL(a, v)                                                \
    (exception = exception ? exception : beetle_store_cell((a), (v)))
#define LOAD_BYTE(a)                                                    \
    ((exception = exception ? exception : beetle_load_byte((a), &byte)), byte)
#define STORE_BYTE(a, v)                                                \
    (exception = exception ? exception : beetle_store_byte((a), (v)))
#define PUSH(v)                                 \
    (SP -= CELL_W, STORE_CELL(SP, (v)))
#define POP                                     \
    (SP += CELL_W, LOAD_CELL(SP - CELL_W))
#define PUSH_DOUBLE(ud)                         \
    PUSH((UCELL)(ud & CELL_MASK));              \
    PUSH((UCELL)((ud >> CELL_BIT) & CELL_MASK))
#define POP_DOUBLE                              \
    (SP += CELL_W * 2, (UCELL)LOAD_CELL(SP - CELL_W), temp |                 \
     ((DUCELL)(UCELL)_LOAD_CELL(SP - 2 * CELL_W, temp2) << CELL_BIT))
#define PUSH_RETURN(v)                          \
    (RP -= CELL_W, STORE_CELL(RP, (v)))
#define POP_RETURN                              \
    (RP += CELL_W, LOAD_CELL(RP - CELL_W))

uint8_t *native_address_range_in_one_area(UCELL start, UCELL length, bool writable);

/* Align a Beetle address */
#define ALIGNED(a) ((a + CELL_W - 1) & (-CELL_W))

/* Check whether a Beetle address is aligned */
#define IS_ALIGNED(a)     (((a) & (CELL_W - 1)) == 0)

/* Portable arithmetic right shift (the behaviour of >> on signed
   quantities is implementation-defined in C99). */
#define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (CELL_BIT - p)))

#define NEXT (exception = (EP += CELL_W, beetle_load_cell(EP - CELL_W, &A)))


#endif
