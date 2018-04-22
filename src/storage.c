// Allocate storage for the registers and memory.
//
// (c) Reuben Thomas 1994-2018

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include "gl_avltree_list.h"
#include "gl_list.h"

#include "beetle.h"
#include "beetle_aux.h"
#include "beetle_private.h"


// Beetle's registers

UCELL EP;
BYTE I;
CELL A;
UCELL SP, RP;
UCELL HASHS = DEFAULT_STACK_SIZE;
UCELL S0, R0;
UCELL HASHR = DEFAULT_STACK_SIZE;
UCELL THROW;
UCELL MEMORY;
UCELL BAD;
UCELL NOT_ADDRESS;


// Memory allocation and mapping
static gl_list_t mem_areas;
static UCELL _mem_here;

typedef struct {
    UCELL start;
    UCELL size;
    uint8_t *ptr;
    bool writable;
} Mem_area;

static int cmp_mem_area(const void *a_, const void *b_)
{
    const Mem_area *a = (const Mem_area *)a_, *b = (const Mem_area *)b_;
    if (a->start + a->size - 1 < b->start)
        return -1;
    else if (a->start > b->start + b->size - 1)
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

// Given a range of addresses, return Mem_area corresponding to some address
// in that range.
// This is used a) to find the area for a particular cell;
//              b) to test whether part of a range has already been allocated
static Mem_area *mem_range(UCELL start, UCELL length)
{
    Mem_area a_addr = {start, length, NULL, true};
    gl_list_node_t elt = gl_sortedlist_search(mem_areas, cmp_mem_area, &a_addr);
    return elt ? (Mem_area *)gl_list_node_value(mem_areas, elt) : NULL;
}

#define addr_in_area(a, addr) (a->ptr + ((addr) - a->start))

_GL_ATTRIBUTE_PURE uint8_t *native_address(UCELL addr, bool write)
{
    Mem_area *a = mem_range(addr, 1);
    if (a == NULL || (write && !a->writable))
        return NULL;
    return addr_in_area(a, addr);
}

// Return address of a range iff it falls inside an area
uint8_t *native_address_range_in_one_area(UCELL start, UCELL length, bool write)
{
    Mem_area *a = mem_range(start, 1);
    if (a == NULL || (write && !a->writable) || a->size - (start - a->start) < length)
        return NULL;
    return addr_in_area(a, start);
}

// Map the given native block of memory to Beetle address addr
static bool mem_map(UCELL addr, void *p, size_t n, bool writable)
{
    // Return false if area is too big, or covers already-allocated addresses
    if ((addr > 0 && n > (CELL_MAX - addr + 1)) || mem_range(addr, n) != NULL)
        return false;

    Mem_area *area = malloc(sizeof(Mem_area));
    if (area == NULL)
        return false;
    *area = (Mem_area){addr, n, p, writable};

    gl_list_node_t elt = gl_sortedlist_nx_add(mem_areas, cmp_mem_area, area);
    if (elt == NULL)
        return false;

    return true;
}

UCELL mem_allot(void *p, size_t n, bool writable)
{
    if (!mem_map(_mem_here, p, n, writable))
        return CELL_MASK;

    size_t start = _mem_here;
    _mem_here += n;
    return start;
}

UCELL mem_align(void)
{
    return _mem_here = ALIGNED(_mem_here);
}


// General memory access

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
    uint8_t *ptr = native_address_range_in_one_area(addr, CELL_W, false);
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
    uint8_t *ptr = native_address_range_in_one_area(addr, CELL_W, true);
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
    Mem_area *a = mem_range(FLIP(addr), 1);
    if (a == NULL) {
        NOT_ADDRESS = addr;
        return -9;
    } else if (!a->writable) {
        NOT_ADDRESS = addr;
        return -20;
    }
    *addr_in_area(a, FLIP(addr)) = value;
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
    if (to < from || native_address_range_in_one_area(from, to - from, write) == NULL)
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


// Initialise registers that are not fixed

int init_beetle(CELL *memory, size_t size)
{
    if (memory == NULL)
        return -1;
    MEMORY = size * CELL_W;
    memset(memory, 0, MEMORY);

    _mem_here = 0UL;

    if (mem_areas)
        gl_list_free(mem_areas);
    if ((mem_areas =
         gl_list_nx_create_empty(GL_AVLTREE_LIST, eq_mem_area, NULL, free_mem_area, false))
        == false)
        return -2;

    if (mem_allot(memory, MEMORY, true) == CELL_MASK)
        return -2;

    CELL *d_stack = calloc(HASHS, CELL_W);
    CELL *r_stack = calloc(HASHR, CELL_W);
    if (d_stack == NULL || r_stack == NULL)
        return -2;

    if (!mem_map(DATA_STACK_SEGMENT, d_stack, HASHS, true)
        || !mem_map(RETURN_STACK_SEGMENT, r_stack, HASHR, true))
        return -2;

    EP = 0;
    A = 0;
    S0 = SP = DATA_STACK_SEGMENT;
    R0 = RP = RETURN_STACK_SEGMENT;
    THROW = 0;
    BAD = CELL_MAX;
    NOT_ADDRESS = CELL_MAX;

    return 0;
}
