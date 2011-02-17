/* BEETLE.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 02may95 Loads and runs bobj (the pForth image file).
    0.01 05jun96 Takes the image file from the command line.
    0.02 06jul04 No longer include noecho.c.

    Reuben Thomas


    A minimal shell for Beetle which simply loads and runs the specified
    object file (syntax: beetle <filename>).

*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "beetle.h"	 /* main header */


#define MEMSIZE 16384	/* size of Beetle's memory in cells */

int main(int argc, char *argv[])
{
    long i;
    CELL *mem;
    char *file = argv[1];
    long adr = 16;
    FILE *handle;
    int ret;

    if (file == NULL) {
        printf("Syntax: Beetle must be given a bForth image filename\n");
        exit(1);
    }

    mem = (CELL *)malloc(MEMSIZE * CELL_W);
    for (i = 0; i < MEMSIZE; i++) mem[i] = 0;
    init_beetle((BYTE *)mem, MEMSIZE, 16);
    *THROW = 0;
    A = 0;

    handle = fopen(file, "r");
    load_object(handle, (CELL *)(M0 + adr));
    fclose(handle);

#ifdef unix
    init_keyb();
#endif
    ret = run();
#ifdef unix
    restore_keyb();
#endif


    return ret;
}
