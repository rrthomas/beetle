/* EPFETCHT.C

    (c) Reuben Thomas 1994-2018

    Also uses NEXT.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


unsigned correct[] = { 20, 20, 20 };


int main(void)
{
    int exception = 0; // FIXME
    CELL temp; // FIXME

    init_beetle((CELL *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_EPFETCH);
    end_ass();

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        printf("EP = %u; should be %u\n\n", EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in EpfetchT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    printf("EpfetchT ran OK\n");
    return 0;
}
