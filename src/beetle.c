/* BEETLE.C

    (c) Reuben Thomas 1995-2011

    A minimal shell for Beetle which simply loads and runs the specified
    object file (syntax: beetle <filename>).

*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "beetle.h"      /* main header */


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

    init_keyb();
    ret = run();
    restore_keyb();

    return ret;
}
