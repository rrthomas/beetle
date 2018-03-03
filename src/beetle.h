/* BEETLE.H

    (c) Reuben Thomas 1994-2018

    Header for C Beetle containing all the data structures and interface
    calls specified in the definition of Beetle. This is the header file to
    include in programs using an embedded Beetle.

*/


#ifndef BEETLE_BEETLE
#define BEETLE_BEETLE


#include "config.h"

#include <stdio.h>      /* for the FILE type */
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "intprops.h"
#include "verify.h"


/* Basic types */
typedef uint8_t BYTE;
typedef int32_t CELL;
typedef uint32_t UCELL;
typedef uint64_t DUCELL;
#define CHAR_MASK ((1 << CHAR_BIT) - 1)
#define CELL_BIT (sizeof(CELL_W) * CHAR_BIT)
#define CELL_MASK ((1ULL << CELL_BIT) - 1)

/* Beetle's registers */

/* ENDISM is fixed at compile-time, which seems reasonable, as
   machines rarely change endianness while switched on! */
#ifdef WORDS_BIGENDIAN
#define ENDISM ((BYTE)1)
#else
#define ENDISM ((BYTE)0)
#endif

extern UCELL EP;
extern BYTE I;
extern CELL A;
extern CELL *M0;
extern UCELL MEMORY;
extern UCELL SP, RP;
extern CELL *THROW;     /* 'THROW is not a valid C identifier */
extern UCELL BAD;       /* 'BAD is not a valid C identifier */
extern UCELL NOT_ADDRESS; /* -ADDRESS is not a valid C identifier */
#define CHECKED 1       /* C Beetle makes address checking mandatory */

/* Memory access */

/* Return value is 0 if OK, or exception code for invalid or unaligned
   address. */
int beetle_load_cell(UCELL addr, CELL *value);
int beetle_load_byte(UCELL addr, BYTE *value);
int beetle_store_cell(UCELL addr, CELL value);
int beetle_store_byte(UCELL addr, BYTE value);

CELL beetle_reverse_cell(CELL value);
int beetle_reverse(UCELL start, UCELL length);
int beetle_pre_dma(UCELL from, UCELL to);
int beetle_post_dma(UCELL from, UCELL to);

/* Memory access */
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

/* High memory */
UCELL himem_here(void);
uint8_t *himem_addr(UCELL addr);
UCELL himem_allot(void *p, size_t n);
UCELL himem_align(void);

/* Interface calls */
CELL run(void);
CELL single_step(void);
int load_object(FILE *file, UCELL address);
int save_object(FILE *file, UCELL address, UCELL length);

/* Additional routines, macros, types and quantities provided by C Beetle */
int init_beetle(CELL *c_array, size_t size);
bool register_args(int argc, char *argv[]);

#define B_TRUE ((CELL)0xFFFFFFFF)   /* Beetle's TRUE flag */
#define B_FALSE ((CELL)0)           /* Beetle's FALSE flag */
#define CELL_W 4    /* the width of a cell in bytes */
#define POINTER_W (sizeof(void *) / CELL_W)   /* the width of a machine pointer in cells */

/* A union to allow storage of machine pointers in Beetle's memory */
typedef union {
    CELL cells[POINTER_W];
    void (*pointer)(void);
} CELL_pointer;

/* Align a Beetle address */
#define ALIGNED(a) ((a + CELL_W - 1) & (-CELL_W))

/* Check whether a Beetle address is aligned */
#define IS_ALIGNED(a)     (((a) & (CELL_W - 1)) == 0)

/* Check whether a Beetle address is in main memory */
#define IN_MAIN_MEMORY(a) ((UCELL)(a) < MEMORY)

/* Macro for byte addressing */
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (CELL_W - 1))
#else
#define FLIP(addr) (addr)
#endif

/* Get the native address of a Beetle address */
#define native_address(a)                       \
    (IN_MAIN_MEMORY(a) ?                        \
     FLIP(a) + (BYTE *)M0 :                     \
     himem_addr(FLIP(a)))

/* Address checking */
#define SET_NOT_ADDRESS(a)                      \
    beetle_store_cell(3 * CELL_W, NOT_ADDRESS = (a))

#define CHECK_ADDRESS(a, cond, code, label)     \
    if (!(cond)) {                              \
        SET_NOT_ADDRESS(a);                     \
        exception = code;                       \
        goto label;                             \
    }

#define CHECK_MAIN_MEMORY_ALIGNED(a)                    \
    CHECK_ADDRESS(a, IN_MAIN_MEMORY(a), -9, badadr)     \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, badadr)

#define NEXT (exception = (EP += CELL_W, beetle_load_cell(EP - CELL_W, &A)))

/* Portable arithmetic right shift (the behaviour of >> on signed
   quantities is implementation-defined in C99). */
#define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (CELL_BIT - p)))


#endif
