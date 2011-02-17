/* BEETLE.H

    (c) Reuben Thomas 1994-1995

    Header for C Beetle containing all the data structures and interface
    calls specified in the definition of Beetle. This is the header file to
    include in programs using an embedded Beetle.

    This header relies on bportab.h for machine dependencies. It is itself
    machine-independent.

*/


#ifndef BEETLE_BEETLE
#define BEETLE_BEETLE


#include "bportab.h"    /* machine-dependent types and definitions */
#include <stdio.h>      /* for the FILE type */


/* Beetle's registers, except for ENDISM, which is defined in bportab.h */

extern CELL *EP;        /* note EP is a pointer, not a Beetle address */
extern BYTE I;
extern CELL A;
extern BYTE *M0;
extern CELL MEMORY;
extern CELL *SP, *RP;	/* note RP and SP are pointers, not Beetle addresses */
extern CELL *THROW;     /* 'THROW is not a valid C identifier */
extern CELL BAD;        /* 'BAD is not a valid C identifier */
extern CELL ADDRESS;	/* -ADDRESS is not a valid C identifier */
#define CHECKED 1


/* Interface calls */

CELL run(void);
CELL single_step(void);
int load_object(FILE *file, CELL *address);
int save_object(FILE *file, CELL *address, UCELL length);


/* Additional routines, macros and quantities provided by C Beetle */

int init_beetle(BYTE *b_array, long size, UCELL e0);

#define B_TRUE ((CELL)0xFFFFFFFF)   /* Beetle's TRUE flag */
#define B_FALSE ((CELL)0)           /* Beetle's FALSE flag */
#define CELL_W 4    /* the width of a cell in bytes */

#define NEXT A = *EP++

#endif
