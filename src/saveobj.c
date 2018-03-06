/* SAVEOBJ.C

    (c) Reuben Thomas 1995-2018

    The interface call save_object(file, address, length) : integer.

*/


#include "config.h"

#include <stdio.h>
#include <string.h>
#include "beetle.h"     /* main header */


int save_object(FILE *file, UCELL address, UCELL length)
{
    uint8_t *ptr = native_address_range_in_one_area(address, address + length);
    if (!IS_ALIGNED(address) || ptr == NULL)
        return -1;

    if (fputs("BEETLE", file) == EOF ||
        putc('\0', file) == EOF ||
        putc((char)ENDISM, file) == EOF ||
        fwrite(&length, CELL_W, 1, file) != 1 ||
        fwrite(ptr, CELL_W, length, file) != length)
        return -3;

    return 0;
}
