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
#include <stdint.h>
#include <limits.h>

#include "intprops.h"
#include "verify.h"


/* Types required by CBeetle: BYTE should be an unsigned eight-bit quantity,
   CELL a signed four-byte quantity, and UCELL an unsigned CELL. */
typedef uint8_t BYTE;
typedef int32_t CELL;
typedef uint32_t UCELL;
#define CELL_BIT (sizeof(CELL_W) * CHAR_BIT)

/* Check that int32_t seems to be two's complement. */
verify (TYPE_MINIMUM(CELL) == INT32_MIN);
verify (TYPE_MAXIMUM(CELL) == INT32_MAX);

/* Beetle's registers */

/* ENDISM is fixed at compile-time, which seems reasonable, as
   machines rarely change endianness while switched on! */
#ifdef WORDS_BIGENDIAN
#define ENDISM ((BYTE)1)
#else
#define ENDISM ((BYTE)0)
#endif

extern CELL *EP;        /* note EP is a pointer, not a Beetle address */
extern BYTE I;
extern CELL A;
extern BYTE *M0;
extern UCELL MEMORY;
extern CELL *SP, *RP;	/* note RP and SP are pointers, not Beetle addresses */
extern CELL *THROW;     /* 'THROW is not a valid C identifier */
extern UCELL BAD;       /* 'BAD is not a valid C identifier */
extern UCELL NOT_ADDRESS; /* -ADDRESS is not a valid C identifier */
#define CHECKED 1       /* C Beetle makes address checking mandatory */

/* High memory */
uint8_t *himem_addr(UCELL addr);
UCELL himem_allot(void *p, size_t n);
UCELL himem_align(void);

/* Interface calls */
CELL run(void);
CELL single_step(void);
int load_object(FILE *file, CELL *address);
int save_object(FILE *file, CELL *address, UCELL length);

/* Additional routines, macros, types and quantities provided by C Beetle */
int init_beetle(BYTE *b_array, size_t size, UCELL e0);

#define B_TRUE ((CELL)0xFFFFFFFF)   /* Beetle's TRUE flag */
#define B_FALSE ((CELL)0)           /* Beetle's FALSE flag */
#define CELL_W 4    /* the width of a cell in bytes */
#define POINTER_W (sizeof(void *) / CELL_W)   /* the width of a machine pointer in cells */

/* A union to allow storage of machine pointers in Beetle's memory */
typedef union {
    CELL cells[POINTER_W];
    void (*pointer)(void);
} CELL_pointer;

#define NEXT A = *EP++

/* Macro for byte addressing */
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (CELL_W - 1))
#else
#define FLIP(addr) (addr)
#endif

/* Align a Beetle address */
#define ALIGNED(a) ((a + CELL_W - 1) & (-CELL_W))

/* Portable arithmetic right shift (the behaviour of >> on signed
   quantities is implementation-defined in C99). */
#define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (CELL_BIT - p)))

/* Division macros */
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SGN(x) ((x) > 0 ? 1 : -1)  /* not a proper sign function! */

#define FDIV(a, b) ((a) / (b) - ((((a) ^ (b)) < 0) && ((a) % (b)) != 0))
#define FMOD(a, b, t) (t = (a) % (b), (((a) ^ (b)) >= 0 || t == 0)) ? t : \
  SGN(b) * (ABS(b)-ABS(t))


#endif
