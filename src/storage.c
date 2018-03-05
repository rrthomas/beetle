/* STORAGE.C

    (c) Reuben Thomas 1994-2018

    Allocate storage for the registers and memory.

*/


#include "config.h"

#include <stdlib.h>
#include "gl_avltree_list.h"
#include "gl_list.h"

#include "beetle.h"	/* main header */


/* Beetle's registers */

UCELL EP;
BYTE I;
CELL A;
CELL *M0;
UCELL MEMORY;
UCELL SP, RP;
CELL *THROW;	/* 'THROW is not a valid C identifier */
UCELL BAD;	/* 'BAD is not a valid C identifier */
UCELL NOT_ADDRESS; /* -ADDRESS is not a valid C identifier */


/* General memory access */

int beetle_load_cell(UCELL addr, CELL *val)
{
    if (!IS_ALIGNED(addr)) {
        SET_NOT_ADDRESS(addr);
        return -23;
    }

    if (IN_MAIN_MEMORY(addr)) {
        *val = *(CELL *)native_address(addr);
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
        *value = *native_address(addr);
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
        *(CELL *)native_address(addr) = value;
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
        *native_address(addr) = value;
        return 0;
    }

    uint8_t *ptr = himem_addr(FLIP(addr));
    if (ptr == NULL)
        return -9;
    *ptr = value;
    return 0;
}


_GL_ATTRIBUTE_CONST CELL beetle_reverse_cell(CELL value)
{
    CELL res = 0;
    for (unsigned i = 0; i < CELL_W / 2; i++) {
        unsigned lopos = CHAR_BIT * i;
        unsigned hipos = CHAR_BIT * (CELL_W - 1 - i);
        unsigned move = hipos - lopos;
        res |= ((((UCELL)value) & (CHAR_MASK << hipos)) >> move)
            | ((((UCELL)value) & (CHAR_MASK << lopos)) << move);
    }
    return res;
}

int beetle_reverse(UCELL start, UCELL length)
{
    int ret = 0;
    for (UCELL i = start; ret == 0 && i < start + length * CELL_W; i += CELL_W) {
        CELL c;
        ret = beetle_load_cell(i, &c)
            || beetle_store_cell(i, beetle_reverse_cell(c));
    }
    return ret;
}

int beetle_pre_dma(UCELL from, UCELL to)
{
    int exception = 0;

    from &= -CELL_W;
    to = ALIGNED(to);
    if (to < from)
        exception = -1;
    CHECK_MAIN_MEMORY_ALIGNED(from);
    CHECK_MAIN_MEMORY_ALIGNED(to);
    if (exception == 0 && ENDISM)
        beetle_reverse(from, to - from);

 badadr:
    return exception;
}

int beetle_post_dma(UCELL from, UCELL to)
{
    return beetle_pre_dma(from, to);
}


/* High memory */
#define HIMEM_START 0x80000000UL // #define so they are constant expressions
#define HIMEM_SIZE  0x80000000UL
gl_list_t himem_areas;
static UCELL _himem_here = HIMEM_START;

typedef struct {
    UCELL start;
    UCELL size;
    uint8_t *ptr;
} Himem_area;

static int cmp_himem_area(const void *a_, const void *b_)
{
    const Himem_area *a = (const Himem_area *)a_, *b = (const Himem_area *)b_;
    if (a->start + a->size <= b->start)
        return -1;
    else if (a->start >= b->start + b->size)
        return 1;
    return 0;
}

static bool eq_himem_area(const void *a_, const void *b_)
{
    return cmp_himem_area(a_, b_) == 0;
}

static void free_himem_area(const void *a)
{
    free((void *)a);
}

static bool himem_init(void)
{
    return (himem_areas =
            gl_list_nx_create_empty(GL_AVLTREE_LIST, eq_himem_area, NULL, free_himem_area, false));
}

_GL_ATTRIBUTE_PURE UCELL himem_here(void)
{
    return _himem_here;
}

_GL_ATTRIBUTE_PURE uint8_t *himem_addr(UCELL addr)
{
    Himem_area a_addr = {addr, CELL_W, NULL};
    gl_list_node_t elt = gl_sortedlist_search(himem_areas, cmp_himem_area, &a_addr);
    if (elt != NULL) {
        Himem_area *a = (Himem_area *)gl_list_node_value(himem_areas, elt);
        if (a != NULL)
            return a->ptr + (addr - a->start);
    }
    return NULL;
}

UCELL himem_allot(void *p, size_t n)
{
    /* Return 0 if not enough room */
    if (n > UINT32_MAX / 2 ||
        (_himem_here - HIMEM_SIZE) + n > HIMEM_SIZE)
        return 0;

    size_t start = _himem_here;
    Himem_area *start_a = malloc(sizeof(Himem_area));
    if (start_a == NULL)
        return 0;
    *start_a = (Himem_area){_himem_here, n, p};
    gl_list_node_t elt = gl_sortedlist_nx_add(himem_areas, cmp_himem_area, start_a);
    if (elt == NULL)
        return 0;
    _himem_here += n;
    return start;
}

UCELL himem_align(void)
{
    return _himem_here = ALIGNED(_himem_here);
}


/* Initialise registers that are not fixed */

int init_beetle(CELL *c_array, size_t size)
{
    if (c_array == NULL || size < 4)
        return -1;

    M0 = c_array;
    EP = 16;
    MEMORY = size * CELL_W;
    beetle_store_cell(1 * CELL_W, MEMORY);
    SP = MEMORY - 0x100;
    RP = size * CELL_W;
    THROW = (CELL *)native_address(0);
    beetle_store_cell(2 * CELL_W, BAD = 0xFFFFFFFF);
    SET_NOT_ADDRESS(0xFFFFFFFF);
    if (!himem_init())
        return -2;

    return 0;
}
