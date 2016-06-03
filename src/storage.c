/* STORAGE.C

    (c) Reuben Thomas 1994-2016

    Allocate storage for the registers and memory.

*/


#include <stdlib.h>
#include "beetle.h"	/* main header */


/* Beetle's registers */

CELL *EP;
BYTE I;
CELL A;
BYTE *M0;
UCELL MEMORY;
CELL *SP, *RP;
CELL *THROW;	/* 'THROW is not a valid C identifier */
UCELL BAD;	/* 'BAD is not a valid C identifier */
UCELL ADDRESS;	/* -ADDRESS is not a valid C identifier */


/* Initialise registers that are not fixed */

int init_beetle(BYTE *b_array, size_t size, UCELL e0)
{
    if (e0 & 3 || e0 >= size * CELL_W)
      return 1;

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
