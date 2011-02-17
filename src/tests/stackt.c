/* STACKT.C

    Vrsn  Date   Comment
    ----|-------|-----------------------------------------------------------
    0.00 14nov94 Test DUP, DROP, SWAP, OVER, ROT, -ROT, TUCK and NIP.
    0.01 16nov94 Also test PICK and ROLL.
    0.02 17nov94 Also test ?DUP, >R, R> and R@; use functions from debug.h.
    0.03 18nov94 Finished tidying up to work with debug.h, and included
    	    	 disassembly of current instruction.
    0.04 22nov94 Changed to work with new version of debug.h.
    0.05 23nov94 Removed spurious variable from main.
    0.06 28nov94 Changed reference to b_mem to one to M0.
    0.07 30nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information. Modified to give a return value
    	    	 from main.
    0.08 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.
    0.09 28feb95 Removed printf format error.

    Reuben Thomas


    Test the stack operators. Also uses the 0 and NEXT instructions.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


char *correct[] = { "1 2 3", "1 2 3 3", "1 2 3", "1 3 2", "1 3 2 3", "1 2 3 3",
     "1 3 2 3", "1 3 3 2 3", "1 3 3 3", "2 1 1", "2 1 2", "2 1 2 2", "1 2 2",
     "1 2 2 2", "2 2 1", "2 2", "2 2 1", "2 2 1 1", "2 2 1 1 1", "2 2 1 1 1 0",
     "2 2 1 1 1 0"};


int main(void)
{
    int i, first;

    init_beetle((BYTE *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    *--SP = 1; *--SP = 2; *--SP = 3;	/* initialise the stack */

    start_ass();
    ass(O_DUP); ass(O_DROP); ass(O_SWAP); ass(O_OVER);
    ass(O_ROT); ass(O_NROT); ass(O_TUCK); ass(O_NIP);
    ass(O_PICK); ass(O_PICK); ass(O_DUP); ass(O_ROLL);
    ass(O_DUP); ass(O_ROLL); ass(O_TOR); ass(O_RFETCH);
    ass(O_RFROM); ass(O_QDUP); ass(O_ZERO); ass(O_QDUP);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i < 10; i++) {
#ifdef B_DEBUG
        show_data_stack();  printf("Correct stack: %s\n\n", correct[i - i / 5]);
#endif
        if (strcmp(correct[i - i / 5], val_data_stack())) {
    	    printf("Error in StackT: EP = %ld\n", val_EP());
    	    exit(1);
    	}
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
    }

    SP = S0;	/* reset stack */
    *--SP = 2; *--SP = 1; *--SP = 0;	/* initialise the stack */
#ifdef B_DEBUG
    printf("Next stack is wrong!\n");
#endif

    first = i;
    for (; i <= instrs; i++) {
#ifdef B_DEBUG
        show_data_stack();  printf("Correct stack: %s\n\n", correct[i - i / 5]);
#endif
        if (strcmp(correct[i - i / 5], val_data_stack()) && i != first) {
    	    printf("Error in StackT: EP = %ld\n", val_EP());
    	    exit(1);
    	}
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
    }

    printf("StackT ran OK\n");
    return 0;
}
