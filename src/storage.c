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
UCELL SP, RP;
UCELL THROW;	/* 'THROW is not a valid C identifier */
UCELL MEMORY;   // Size of main memory
UCELL BAD;	/* 'BAD is not a valid C identifier */
UCELL NOT_ADDRESS; /* -ADDRESS is not a valid C identifier */


/* Memory allocation and mapping */
gl_list_t mem_areas;
static UCELL _mem_here = 0UL;

typedef struct {
    UCELL start;
    UCELL size;
    uint8_t *ptr;
    bool writable;
} Mem_area;

static int cmp_mem_area(const void *a_, const void *b_)
{
    const Mem_area *a = (const Mem_area *)a_, *b = (const Mem_area *)b_;
    if (a->start + a->size <= b->start)
        return -1;
    else if (a->start >= b->start + b->size)
        return 1;
    return 0;
}

static bool eq_mem_area(const void *a_, const void *b_)
{
    return cmp_mem_area(a_, b_) == 0;
}

static void free_mem_area(const void *a)
{
    free((void *)a);
}

_GL_ATTRIBUTE_PURE UCELL mem_here(void)
{
    return _mem_here;
}

static Mem_area *mem_area(UCELL addr)
{
    Mem_area a_addr = {addr, CELL_W, NULL, true};
    gl_list_node_t elt = gl_sortedlist_search(mem_areas, cmp_mem_area, &a_addr);
    return elt ? (Mem_area *)gl_list_node_value(mem_areas, elt) : NULL;
}

#define addr_in_area(a, addr) (a->ptr + (addr - a->start))

_GL_ATTRIBUTE_PURE uint8_t *native_address(UCELL addr, bool write)
{
    Mem_area *a = mem_area(addr);
    if (a == NULL || (write && !a->writable))
        return NULL;
    return addr_in_area(a, addr);
}

// Return address of a range [start,end) iff it falls inside an area
uint8_t *native_address_range_in_one_area(UCELL start, UCELL end, bool write)
{
    Mem_area *a = mem_area(start);
    if (a == NULL || (write && !a->writable) || end > a->start + a->size)
        return NULL;
    return addr_in_area(a, start);
}

UCELL mem_allot(void *p, size_t n, bool writable)
{
    /* Return highest possible address if not enough room */
    if (n > (CELL_MAX - _mem_here))
        return CELL_MASK;

    size_t start = _mem_here;
    Mem_area *start_a = malloc(sizeof(Mem_area));
    if (start_a == NULL)
        return 0;
    *start_a = (Mem_area){_mem_here, n, p, writable};
    gl_list_node_t elt = gl_sortedlist_nx_add(mem_areas, cmp_mem_area, start_a);
    if (elt == NULL)
        return 0;
    _mem_here += n;
    return start;
}

UCELL mem_align(void)
{
    return _mem_here = ALIGNED(_mem_here);
}


/* General memory access */

// Macro for byte addressing
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (CELL_W - 1))
#else
#define FLIP(addr) (addr)
#endif

int beetle_load_cell(UCELL addr, CELL *value)
{
    if (!IS_ALIGNED(addr)) {
        NOT_ADDRESS = addr;
        return -23;
    }

    // Aligned access to a single memory area
    uint8_t *ptr = native_address_range_in_one_area(addr, addr + CELL_W, false);
    if (ptr != NULL && IS_ALIGNED((size_t)ptr)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
        *value = *(CELL *)ptr;
#pragma GCC diagnostic pop
        return 0;
    }

    // Awkward access
    *value = 0;
    for (unsigned i = 0; i < CELL_W; i++, addr++) {
        ptr = native_address(addr, false);
        if (ptr == NULL) {
            NOT_ADDRESS = addr;
            return -9;
        }
        ((BYTE *)value)[ENDISM ? CELL_W - i : i] = *ptr;
    }
    return 0;
}

int beetle_load_byte(UCELL addr, BYTE *value)
{
    uint8_t *ptr = native_address(FLIP(addr), false);
    if (ptr == NULL)
        return -9;
    *value = *ptr;
    return 0;
}

int beetle_store_cell(UCELL addr, CELL value)
{
    if (!IS_ALIGNED(addr)) {
        NOT_ADDRESS = addr;
        return -23;
    }

    // Aligned access to a single memory allocation
    uint8_t *ptr = native_address_range_in_one_area(addr, addr + CELL_W, true);
    if (ptr != NULL && IS_ALIGNED((size_t)ptr)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
        *(CELL *)ptr = value;
#pragma GCC diagnostic pop
        return 0;
    }

    // Awkward access
    int exception = 0;
    for (unsigned i = 0; exception == 0 && i < CELL_W; i++)
        exception = beetle_store_byte(addr + i, value >> ((ENDISM ? CELL_W - i : i) * CHAR_BIT));
    return exception;
}

int beetle_store_byte(UCELL addr, BYTE value)
{
    Mem_area *a = mem_area(FLIP(addr));
    if (a == NULL) {
        NOT_ADDRESS = addr;
        return -9;
    } else if (!a->writable) {
        NOT_ADDRESS = addr;
        return -20;
    }
    *addr_in_area(a, addr) = value;
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
int beetle_pre_dma(UCELL from, UCELL to, bool write)
{
    int exception = 0;

    from &= -CELL_W;
    to = ALIGNED(to);
    if (to < from || native_address_range_in_one_area(from, to, write) == NULL)
        exception = -1;
    CHECK_ALIGNED(from);
    CHECK_ALIGNED(to);
    if (exception == 0 && ENDISM)
        exception = beetle_reverse(from, to - from);

 badadr:
    return exception;
}
#pragma GCC diagnostic pop

int beetle_post_dma(UCELL from, UCELL to)
{
    return beetle_pre_dma(from, to, false);
}


/* Initialise registers that are not fixed */

int init_beetle(CELL *c_array, size_t size)
{
    if (c_array == NULL)
        return -1;

    EP = 16;

    if ((mem_areas =
         gl_list_nx_create_empty(GL_AVLTREE_LIST, eq_mem_area, NULL, free_mem_area, false))
        == false)
        return -2;

    if (mem_allot(&THROW, CELL_W, true) == CELL_MASK
        || mem_allot(&MEMORY, CELL_W, false) == CELL_MASK
        || mem_allot(&BAD, CELL_W, false) == CELL_MASK
        || mem_allot(&NOT_ADDRESS, CELL_W, false) == CELL_MASK
        || mem_allot(c_array, size * CELL_W, true) == CELL_MASK)
        return -2;

    size += 4; // FIXME
    MEMORY = size * CELL_W; // FIXME: move to mem_allot
    SP = size * CELL_W - 0x100;
    RP = size * CELL_W;
    BAD = 0xFFFFFFFF;
    NOT_ADDRESS = 0xFFFFFFFF;

    return 0;
}
