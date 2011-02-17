/* BEETLE.H

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 07nov94
    0.01 09nov94 Address checking made mandatory; b_mem made an array,
    	    	 rather than a pointer, EP made a pointer.
    0.02 10nov94 single_step's declaration corrected.
    0.03 11nov94 SP and RP made pointers; b_mem made a pointer again; CHECKED
    	    	 temporarily made 0.
    0.04 17nov94 Ensured this file can only be included once.
    0.05 23nov94 Changed THROW to a pointer (it must be phyiscally held in
    	    	 Beetle's address space according to the specification).
    0.06 25nov94 Changed return types of single_step and run to long.
    0.07 28nov94 Removed declaration of b_mem, which is now exactly the same as
    	    	 M0.
    0.08 01dec94 Changed return type of run and single_step to CELL.
    0.09 13jan95 Removed save_standalone, which is not implemented, and the LIST
                 datatype which supports it.
    0.10 24jan95 Changed types of address arguments to load_object and
    	    	 save_object to CELL *.
    0.11 17feb95 Moved much of bintern.h here:
    	0.00 09nov94
    	0.01 10nov94 NEXT made more efficient.
    	0.02 18nov94 Added QCELL_W, a string containing the cell width.
    	0.03 22nov94 Added B_TRUE and B_FALSE, Beetle's values for TRUE and
    	    	     FALSE.
    	0.04 25nov94 Added lib, the function which provides the standard library
    	    	     calls.
    	0.05 17feb95 Changed prototype of init_regs (now init_beetle) to match
    	    	     modified storage.c. Moved most of the header into beetle.h,
    	    	     and QCELL_W into btests.h.
    	0.06 25mar95 lib() no longer exists. Header now empty, so not included.
    	     05jun96 Header removed altogether, and documentation placed here.
    0.12 26feb95 Changed type of length in save_object to UCELL. Typing on
    	    	 CHECKED removed to allow it to be used in #ifs.
    0.13 23mar95 Changed type of e0 in init_beetle to UCELL.
    0.14 24apr95 Removed load_library prototype (not implemented).

    Reuben Thomas


    Header for C Beetle containing all the data structures and interface
    calls specified in the definition of Beetle. This is the header file to
    include in programs using an embedded Beetle.

    This header relies on bportab.h for machine dependencies. It is itself
    machine-independent.

*/


#ifndef BEETLE_BEETLE
#define BEETLE_BEETLE


#include "bportab.h"  	/* machine-dependent types and definitions */
#include <stdio.h>  	/* for the FILE type */


/* Beetle's registers, except for ENDISM, which is defined in bportab.h */

extern CELL *EP;    	/* note EP is a pointer, not a Beetle address */
extern BYTE I;
extern CELL A;
extern BYTE *M0;
extern CELL MEMORY;
extern CELL *SP, *RP;	/* note RP and SP are pointers, not Beetle addresses */
extern CELL *THROW;  	/* 'THROW is not a valid C identifier */
extern CELL BAD;    	/* 'BAD is not a valid C identifier */
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
#define B_FALSE ((CELL)0)   	    /* Beetle's FALSE flag */
#define CELL_W 4    /* the width of a cell in bytes */

#define NEXT A = *EP++

#endif
