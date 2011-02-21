/* BEETLE.H

    (c) Reuben Thomas 1994-2011

    Header for C Beetle containing all the data structures and interface
    calls specified in the definition of Beetle. This is the header file to
    include in programs using an embedded Beetle.

*/


#ifndef BEETLE_BEETLE
#define BEETLE_BEETLE


#include "config.h"

#include <stdio.h>      /* for the FILE type */
#include <stdint.h>


/* Types required by CBeetle: BYTE should be an unsigned eight-bit quantity,
   CELL a signed four-byte quantity, and UCELL an unsigned CELL. */
typedef uint8_t BYTE;
typedef int32_t CELL;
typedef uint32_t UCELL;


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
extern UCELL ADDRESS;	/* -ADDRESS is not a valid C identifier */
#define CHECKED 1


/* Interface calls */
CELL run(void);
CELL single_step(void);
int load_object(FILE *file, CELL *address);
int save_object(FILE *file, CELL *address, UCELL length);

/* Additional routines, macros and quantities provided by C Beetle */
int init_beetle(BYTE *b_array, size_t size, UCELL e0);

#define B_TRUE ((CELL)0xFFFFFFFF)   /* Beetle's TRUE flag */
#define B_FALSE ((CELL)0)           /* Beetle's FALSE flag */
#define CELL_W 4    /* the width of a cell in bytes */

#define NEXT A = *EP++

/* Macro for byte addressing */
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ 3)
#else
#define FLIP(addr) (addr)
#endif

/* Portable arithmetic right shift (the behaviour of >> on signed
   quantities is implementation-defined in C99). */
#define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (32 - p)))

/* Division macros */
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SGN(x) ((x) > 0 ? 1 : -1 )  /* not a proper sign function! */

#define DIV(a, b) ((a) / (b) - ((((a) ^ (b)) < 0) && ((a) % (b)) != 0))
#define MOD(a, b, t) (t = (a) % (b), (((a) ^ (b)) >= 0 || t == 0)) ? t : \
  SGN(b) * (ABS(b)-ABS(t))
#define SDIV(a, b) (a / b)
#define SMOD(a, b, t) (a % b)

/* A macro to call the function pointed to by the top item(s) on Beetle's
   data stack and increment the stack pointer to drop the pointer */
/* FIXME: This will only work on 32-bit machines. */
#define LINK SP++; (*(void (*)(void))*(SP - 1))()


#endif
