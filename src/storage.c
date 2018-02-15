/* STORAGE.C

    (c) Reuben Thomas 1994-2018

    Allocate storage for the registers and memory.

*/


#include "config.h"


#include <stdlib.h>
#include "beetle.h"	/* main header */


/* Beetle's registers */

CELL *EP;
BYTE I;
CELL A;
CELL *M0;
UCELL MEMORY;
CELL *SP, *RP;
CELL *THROW;	/* 'THROW is not a valid C identifier */
UCELL BAD;	/* 'BAD is not a valid C identifier */
UCELL NOT_ADDRESS; /* -ADDRESS is not a valid C identifier */


/* Initialise registers that are not fixed */

int init_beetle(CELL *c_array, size_t size, UCELL e0)
{
    if (!IS_ALIGNED(e0) || e0 >= size * CELL_W)
        return 1;

    M0 = c_array;
    EP = M0 + e0 / CELL_W;
    MEMORY = size * CELL_W;
    SP = M0 + (size - 0x40);
    RP = M0 + size;
    THROW = M0;
    BAD = 0xFFFFFFFF;
    NOT_ADDRESS = 0xFFFFFFFF;

    M0[1] = MEMORY;
    M0[2] = BAD;
    M0[3] = NOT_ADDRESS;

    return 0;
}


/* High memory */
static UCELL HIMEM_START = 0x80000000UL;
static UCELL HIMEM_SIZE = 0x80000000UL;
#define HIMEM_MAX_AREAS 256
static uint8_t *himem_area[HIMEM_MAX_AREAS];
static UCELL himem_size[HIMEM_MAX_AREAS];
static UCELL himem_areas = 0;
static UCELL _himem_here = 0x80000000UL;

_GL_ATTRIBUTE_PURE UCELL himem_here(void)
{
    return _himem_here;
}

_GL_ATTRIBUTE_PURE uint8_t *himem_addr(UCELL addr)
{
    if (addr < HIMEM_START || addr > _himem_here)
        return NULL;
    UCELL start = HIMEM_START;
    for (UCELL i = 0; i < himem_areas; i++) {
        if (addr < start + himem_size[i])
            return himem_area[i] + (addr - start);
        start += himem_size[i];
    }
    return NULL; // Should never reach here
}

UCELL himem_allot(void *p, size_t n)
{
    /* Return 0 if not enough room */
    if (himem_areas == HIMEM_MAX_AREAS ||
        n > UINT32_MAX / 2 ||
        (_himem_here - HIMEM_SIZE) + n > HIMEM_SIZE)
        return 0;

    size_t start = _himem_here;
    himem_area[himem_areas] = p;
    himem_size[himem_areas] = n;
    _himem_here += n;
    himem_areas++;
    return start;
}

UCELL himem_align(void)
{
    struct {
        uint8_t byte1 __attribute__ ((__aligned__));
        uint8_t byte2 __attribute__ ((__aligned__));
    } test;
    size_t max_alignment = &test.byte2 - &test.byte1;

    return _himem_here = (_himem_here + max_alignment - 1) & (-max_alignment);
}
