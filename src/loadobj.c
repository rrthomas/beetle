/* LOADOBJ.C

    (c) Reuben Thomas 1995-2018

    The interface call load_object(file, address) : integer.

*/


#include "config.h"

#include <stdio.h>
#include <string.h>
#include "beetle.h"	/* main header */


int load_object(FILE *file, CELL *address)
{
    char magic[8];
    if (fread(&magic[0], 1, 7, file) != 7)
        return -3;
    magic[7] = '\0';
    if (strcmp(magic, "BEETLE"))
        return -2;

    uint8_t endism;
    if (fread(&endism, 1, 1, file) != 1)
        return -3;
    if (endism != 0 && endism != 1)
        return -2;
    int reversed = endism ^ ENDISM;

    UCELL length = 0;
    if (fread(&length, 1, CELL_W, file) != CELL_W)
        return -3;
    if (reversed)
        beetle_reverse((CELL *)&length, 1);
    if ((((address - M0) + length) * CELL_W > MEMORY) ||
        ((UCELL)(address - M0) * CELL_W) == MEMORY)
        return -1;

    if (fread(address, CELL_W, length, file) != length)
        return -3;
    if (reversed)
        beetle_reverse(address, length);

    return 0;
}
