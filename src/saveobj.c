/* SAVEOBJ.C

    (c) Reuben Thomas 1995-2011

    The interface call save_object(file, address, length) : integer.

*/


#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "beetle.h"     /* main header */


jmp_buf env;

static void put(int c, FILE *fp)
{
    int t = putc(c, fp);
    if (t == EOF) longjmp(env, -3);
}

int save_object(FILE *file, CELL *address, UCELL length)
{
    char magic[] = "BEETLE\0";
    int err = 0;
    UCELL i;

    if (length > (UCELL)0x3fffffff) { err = -1; goto error; }

    if ((err = setjmp(env)) == 0) {
        if ((UCELL)(address - (CELL *)M0) > MEMORY ||
            ((address - (CELL *)M0) + length) * CELL_W > MEMORY) {
            err = -1;
            goto error;
        }

        for (i = 0; i < 7; i++)
            put(magic[i], file);
        put(ENDISM, file);
        for (i = 0; i < CELL_W; i++)
            put(((BYTE *)&length)[i], file);
        for (i = 0; i < length * CELL_W; i++)
            put(((BYTE *)address)[i], file);

        return 0;
    } else {
error:  return err;
    }
}
