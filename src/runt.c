/* RUNT.C

    (c) Reuben Thomas 1995

    Test that run works, and that the return value of the HALT instruction is
    correctly returned.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


int main(void)
{
    int i;
    CELL ret;

    i = init_beetle((BYTE *)malloc(1024), 256, 16);
    if (i != 0) {
        printf("Error in RunT: init_beetle with valid parameters failed\n");
        exit(1);
    }
    for (i = 0; i < 1024; i++) M0[i] = 0;

    here = (CELL *)(M0 + 52);
    start_ass();
    ass(O_LITERALI); ilit(37);
    ass(O_HALT);
    end_ass();

    NEXT;
    ret = run();

    if (B_DEBUG)
      printf("Return value should be 37 and is %ld\n", ret);
    if (ret != 37) {
        printf("Error in RunT: incorrect return value from run\n");
        exit(1);
    }

    if (B_DEBUG)
      printf("EP should now be 56\n");
    if (val_EP() != 60) {
        printf("Error in RunT: EP = %ld\n", val_EP());
        exit(1);
    }

    printf("RunT ran OK\n");
    return 0;
}
