// The interface call load_object(file, address) : integer.
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "beetle.h"
#include "beetle_aux.h"


int load_object(FILE *file, UCELL address)
{
    if (!IS_ALIGNED(address))
        return -1;

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
        length = (UCELL)beetle_reverse_cell((CELL)length);

    uint8_t *ptr = native_address_range_in_one_area(address, length * CELL_W, true);
    if (ptr == NULL)
        return -1;

    if (fread(ptr, CELL_W, length, file) != length)
        return -3;
    if (reversed)
        beetle_reverse(address, length);

    return 0;
}
