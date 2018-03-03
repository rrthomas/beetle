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


int exception = 0;
CELL temp;


int main(void)
{
    int i = init_beetle((CELL *)calloc(1024, 1), 256);
    if (i != 0) {
        printf("Error in RunT: init_beetle with valid parameters failed\n");
        exit(1);
    }

    here = 52;
    start_ass();
    ass(O_LITERALI); ilit(37);
    ass(O_HALT);
    end_ass();

    NEXT;
    CELL ret = run();

    printf("Return value should be 37 and is %"PRId32"\n", ret);
    if (ret != 37) {
        printf("Error in RunT: incorrect return value from run\n");
        exit(1);
    }

    printf("EP should now be 56\n");
    if (EP != 60) {
        printf("Error in RunT: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("RunT ran OK\n");
    return 0;
}
