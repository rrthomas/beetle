/* STEPT.C

    (c) Reuben Thomas 1994-2018

    Test that single_step works, and that address alignment and bounds
    checking is properly performed on EP.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "debug.h"      /* debugging functions */


int main(void)
{
    int exception = 0; // FIXME
    CELL temp; // FIXME

    int i;

    i = init_beetle((CELL *)NULL, 1, 1);
    printf("init_beetle((CELL *)NULL, 1, 1) should return 1; returns: %d\n", i);
    if (i != 1) {
        printf("Error in StepT: init_beetle with invalid parameters "
            "succeeded\n");
        exit(1);
    }
    i = init_beetle((CELL *)NULL, 1, 4);
    printf("init_beetle((CELL *)NULL, 1, 4) should return 1; returns: %d\n", i);
    if (i != 1) {
        printf("Error in StepT: init_beetle with invalid parameters "
            "succeeded\n");
        exit(1);
    }

    i = init_beetle((CELL *)calloc(1024, 1), 256, 16);
    if (i != 0) {
        printf("Error in StepT: init_beetle with valid parameters failed\n");
        exit(1);
    }

    NEXT;

    for (i = 0; i < 10; i++) {
        printf("EP = %u\n", EP);
        single_step();
    }

    printf("EP should now be 60\n");
    if (EP != 60) {
        printf("Error in StepT: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("StepT ran OK\n");
    return 0;
}
