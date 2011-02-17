/* CREATET.C

    (c) Reuben Thomas 1994-2011

    Also uses NEXT.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


int correct[] = { 20, 20, 20 };


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_CREATE);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
        printf("EP = %d; should be %d\n\n", val_EP(), correct[i]);
        if (correct[i] != val_EP()) {
            printf("Error in CreateT: EP = %ld\n", val_EP());
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    printf("CreateT ran OK\n");
    return 0;
}
