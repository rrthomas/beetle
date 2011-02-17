/* STEPT.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 09nov94
    0.01 14nov94 Start value of EP changed to avoid registers stored in low
    	    	 memory. Action of NEXT performed when initialising EP as per
    	    	 specification. Added check message at end.
    0.02 18nov94 Made printing of EP use debug.h function.
    0.03 28nov94 Changed reference to b_mem to one to M0.
    0.03 30nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information. Modified to give a return value
    	    	 from main.
    0.04 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.
    0.05 28feb95 Removed printf format error.
    0.06 24mar95 Added code to test address alignment and bounds checking on EP.

    Reuben Thomas


    Test that single_step works, and that address alignment and bounds
    checking is properly performed on EP.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "debug.h"  	/* debugging functions */


int main(void)
{
    int i;

    i = init_beetle((BYTE *)NULL, 1, 1);
#ifdef B_DEBUG
    printf("init_beetle((BYTE *)NULL, 1, 1) should return 1; returns: %d\n", i);
#endif
    if (i != 1) {
        printf("Error in StepT: init_beetle with invalid parameters "
            "succeeded\n");
        exit(1);
    }
    i = init_beetle((BYTE *)NULL, 1, 4);
#ifdef B_DEBUG
    printf("init_beetle((BYTE *)NULL, 1, 4) should return 1; returns: %d\n", i);
#endif
    if (i != 1) {
        printf("Error in StepT: init_beetle with invalid parameters "
            "succeeded\n");
        exit(1);
    }

    i = init_beetle((BYTE *)malloc(1024), 256, 16);
    if (i != 0) {
        printf("Error in StepT: init_beetle with valid parameters failed\n");
        exit(1);
    }
    for (i = 0; i < 1024; i++) M0[i] = 0;

    NEXT;

    for (i = 0; i < 10; i++) {
#ifdef B_DEBUG
        printf("EP = %d\n", val_EP());
#endif
        single_step();
    }

#ifdef B_DEBUG
    printf("EP should now be 56\n");
#endif
    if (val_EP() != 60) {
        printf("Error in StepT: EP = %ld\n", val_EP());
        exit(1);
    }

    printf("StepT ran OK\n");
    return 0;
}
