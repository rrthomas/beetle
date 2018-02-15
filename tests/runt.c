/* RUNT.C

    (c) Reuben Thomas 1995-2018

    Test that run works, and that the return value of the HALT instruction is
    correctly returned.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


int main(void)
{
    CELL ret;

    int i = init_beetle((CELL *)calloc(1024, 1), 256, 16);
    if (i != 0) {
        printf("Error in RunT: init_beetle with valid parameters failed\n");
        exit(1);
    }

    here = M0 + 52 / CELL_W;
    start_ass();
    ass(O_LITERALI); ilit(37);
    ass(O_HALT);
    end_ass();

    NEXT;
    ret = run();

    printf("Return value should be 37 and is %"PRId32"\n", ret);
    if (ret != 37) {
        printf("Error in RunT: incorrect return value from run\n");
        exit(1);
    }

    printf("EP should now be 56\n");
    if (val_EP() != 60) {
        printf("Error in RunT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    printf("RunT ran OK\n");
    return 0;
}
