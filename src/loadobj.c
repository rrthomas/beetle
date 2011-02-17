/* LOADOBJ.C

    (c) Reuben Thomas 1995-1996

    The interface call load_object(file, address) : integer.

*/


#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "beetle.h"	/* main header */


static void reverse(CELL *start, UCELL length)
{
    UCELL i;

    for (i = 0; i < length; i++)
        start[i] = (CELL)(((UCELL) start[i] << 24) | ((UCELL)start[i] >> 24) |
            (((UCELL)start[i] & 0xff00) << 8) |
            (((UCELL)start[i] & 0xff0000) >> 8));
}


static jmp_buf env;

static int get(FILE *fp)
{
    int t = getc(fp);
    if (t == EOF) longjmp(env, -3);
    return t;
}

int load_object(FILE *file, CELL *address)
{
    char magic[8];
    int endism, reversed, i, err = 0;
    UCELL length = 0;

    if ((err = setjmp(env)) == 0) {
        for (i = 0; i < 7; i++) magic[i] = get(file);
        magic[7] = '\0';
        if (strcmp(magic, "BEETLE")) { err = -2; goto error; }

        endism = get(file);
        if (endism != 0 && endism != 1) { err = -2; goto error; }
        reversed = endism ^ ENDISM;

        for (i = 0; i < CELL_W; i++) length |= get(file) << (8 * i);
        if (endism) reverse((CELL *)&length, 1);
        if ((((address - (CELL *)M0) + length) * CELL_W > MEMORY) ||
            (address - (CELL *)M0) * CELL_W == MEMORY) {
            err = -1;
            goto error;
        }

        for (i = 0; i < length * CELL_W; i++) ((BYTE *)address)[i] = get(file);
        if (reversed) reverse(address, length);

        return 0;
    } else {
error:  return err;
    }
}
