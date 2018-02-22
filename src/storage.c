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
UCELL SP;
CELL *RP;
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
    SP = MEMORY - 256;
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
    // We must have tried to address a gap skipped by himem_align
    return NULL;
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


/* General memory access */

#define SET_NOT_ADDRESS(a)                      \
    M0[3] = NOT_ADDRESS = (a);

int beetle_load_cell(UCELL addr, CELL *val)
{
    if (!IS_ALIGNED(addr)) {
        SET_NOT_ADDRESS(addr);
        return -23;
    }

    if (IN_MAIN_MEMORY(addr)) {
        *val = M0[addr / CELL_W];
        return 0;
    }

    *val = 0;
    for (unsigned i = 0; i < sizeof(CELL); i++, addr++) {
        uint8_t *ptr = himem_addr(addr);
        if (ptr == NULL) {
            SET_NOT_ADDRESS(addr);
            return -9;
        }
        ((BYTE *)val)[ENDISM ? CELL_W - i : i] = *ptr;
    }
    return 0;
}

int beetle_load_byte(UCELL addr, BYTE *value)
{
    if (IN_MAIN_MEMORY(addr)) {
        *value = ((BYTE *)M0)[FLIP(addr)];
        return 0;
    }

    uint8_t *ptr = himem_addr(FLIP(addr));
    if (ptr == NULL)
        return -9;
    *value = *ptr;
    return 0;
}

int beetle_store_cell(UCELL addr, CELL value)
{
    if (!IS_ALIGNED(addr)) {
        SET_NOT_ADDRESS(addr);
        return -23;
    }

    if (IN_MAIN_MEMORY(addr)) {
        M0[addr / CELL_W] = value;
        return 0;
    }

    for (unsigned i = 0; i < sizeof(CELL); i++, addr++) {
        uint8_t *ptr = himem_addr(addr);
        if (ptr == NULL) {
            SET_NOT_ADDRESS(addr);
            return -9;
        }
        *ptr = value >> ((ENDISM ? i : CELL_W - i) * CHAR_BIT);
    }
    return 0;
}

int beetle_store_byte(UCELL addr, BYTE value)
{
    if (IN_MAIN_MEMORY(addr)) {
        ((BYTE *)M0)[FLIP(addr)] = value;
        return 0;
    }

    uint8_t *ptr = himem_addr(FLIP(addr));
    if (ptr == NULL)
        return -9;
    *ptr = value;
    return 0;
}


void beetle_reverse(CELL *start, UCELL length)
{
    for (UCELL i = 0; i < length; i++)
        start[i] = (CELL)(((UCELL) start[i] << 24) | ((UCELL)start[i] >> 24) |
            (((UCELL)start[i] & 0xff00) << 8) |
            (((UCELL)start[i] & 0xff0000) >> 8));
}

int beetle_pre_dma(UCELL from, UCELL to)
{
    int exception = 0;

    from &= -CELL_W;
    to = ALIGNED(to);
    if (to < from)
        exception = -262; // FIXME: Document this!
    CHECK_MAIN_MEMORY_ALIGNED(from);
    CHECK_MAIN_MEMORY_ALIGNED(to);
    if (exception == 0 && ENDISM)
        beetle_reverse(M0 + from / CELL_W, to - from);

 badadr:
    return exception;
}

int beetle_post_dma(UCELL from, UCELL to)
{
    return beetle_pre_dma(from, to);
}