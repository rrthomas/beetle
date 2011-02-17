/* LOGICT.C

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 22nov94 Test INVERT, AND, OR, XOR, LSHIFT, RSHIFT, 1LSHIFT and
    	    	 1RSHIFT.
    0.01 23nov94 Removed spurious variable from main.
    0.02 28nov94 Changed reference to b_mem to one to M0.
    0.03 30nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information. Modified to give a return value
    	    	 from main.
    0.04 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.
    0.05 28feb95 Removed printf format error.

    Reuben Thomas


    Test the logic operators. Also uses the NEXT and -ROT instructions. We
    only test the stack handling and basic correctness of the operators here,
    assuming that if the logic works in one case, it will work in all (if the
    C compiler doesn't implement it correctly, we're in trouble anyway!).

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


char *correct[] = { "-16777216 8 255 8", "-16777216 8 65280",
    "65280 -16777216 8", "65280 16711680", "16776960", "33553920", "16776960",
    "-16776961", "-16776961 1", "-16776961 1 -1", "-16776961 -2", "-16776962"};


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    *--SP = 0xFF000000; *--SP = 8; *--SP = 0xFF; *--SP = 8;

    start_ass();
    ass(O_LSHIFT); ass(O_NROT); ass(O_RSHIFT); ass(O_OR);
    ass(O_LSHIFT1); ass(O_RSHIFT1); ass(O_INVERT); ass(O_ONE);
    ass(O_MONE); ass(O_XOR); ass(O_AND);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
#ifdef B_DEBUG
        show_data_stack();  printf("Correct stack: %s\n\n", correct[i - i / 5]);
#endif
        if (strcmp(correct[i - i / 5], val_data_stack())) {
    	    printf("Error in LogicT: EP = %ld\n", val_EP());
    	    exit(1);
    	}
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
    }

    printf("LogicT ran OK\n");
    return 0;
}
