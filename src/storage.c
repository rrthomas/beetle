/* STORAGE.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 09nov94
    0.01 11nov94 SP and RP changed to pointers; definition of memory vector
                 changed to ensure it is aligned on a 4-byte boundary.
    0.02 22nov94 b_mem is now initialised explicitly to zero (unnecessary,
                 but helpful).
    0.03 23nov94 Changed THROW to a pointer, so that it is phyiscally held
                 in Beetle's address space as required. Made init_regs copy
                 registers to memory as specified, and changed scope of cast
                 in assignments to SP and RP to make them correct.
    0.04 28nov94 Removed b_mem, which was identical with M0.
    0.05 19jan95 Added compiler tests from tests.c to init_regs.
    0.06 17feb95 Parametrised init_regs (now init_beetle), and removed
                 allocation of memory array and now unnecessary reference to
                 bintern.h. init_beetle made to return a value (1 if e0 is not
                 aligned, 0 if it's OK).
    0.07 24feb95 Added code to init_beetle to initialise 'THROW.
    0.08 23mar95 Added code to init_beetle to check whether e0 is in range (and
                 return 1 if not), and changed e0 to type UCELL.
    0.09 17feb11 #include <stdlib.h> for exit.

    Reuben Thomas


    Allocate storage for the registers and memory.

*/


#include <stdlib.h>
#include "beetle.h"	/* main header */
#include "tests.h"	/* compiler tests */


/* Beetle's registers */

CELL *EP;
BYTE I;
CELL A;
BYTE *M0;
CELL MEMORY;
CELL *SP, *RP;
CELL *THROW;	/* 'THROW is not a valid C identifier */
CELL BAD;	/* 'BAD is not a valid C identifier */
CELL ADDRESS;	/* -ADDRESS is not a valid C identifier */


/* Initialise registers that are not fixed */

int init_beetle(BYTE *b_array, long size, UCELL e0)
{
    if (tests()) exit(1);   /* ensure Beetle was compiled properly */
    if (e0 & 3 || e0 >= size * CELL_W) return 1;

    M0 = (BYTE *)b_array;
    EP = (CELL *)(M0 + e0);
    MEMORY = size * CELL_W;
    SP = (CELL *)(M0 + (MEMORY - 0x100));
    RP = (CELL *)(M0 + MEMORY);
    THROW = (CELL *)M0;
    BAD = 0xFFFFFFFF;
    ADDRESS = 0xFFFFFFFF;

    *((CELL *)M0 + 1) = MEMORY;
    *((CELL *)M0 + 2) = BAD;
    *((CELL *)M0 + 3) = ADDRESS;

    return 0;
}
