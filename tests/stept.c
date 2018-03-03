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
    int i = init_beetle((CELL *)calloc(1024, 1), 256);
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
