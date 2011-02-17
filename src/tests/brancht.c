/* BRANCHT.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 23nov94 Test BRANCH, BRANCHI, ?BRANCH, and ?BRANCHI.
    0.01 24nov94 Finished tests for ?BRANCH and ?BRANCHI; added tests for
    	    	 EXECUTE, @EXECUTE, CALL, CALLI and EXIT.
    0.02 28nov94 Changed reference to b_mem to one to M0.
    0.03 29nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information.
    0.04 30nov94 Modified to give a return value from main.
    0.05 17feb95 Modified to work with new storage.c, and to use btests.h rather
    	    	 than bintern.h.
    0.06 28feb95 Removed printf format error.

    Reuben Thomas


    Test the branch instructions. Also uses other instructions with lower
    opcodes than the instructions tested (i.e. those already tested). Doesn't
    test address exception handling, as this is not yet supported. The test
    program contains an infinite loop, but this is only executed once.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


int correct[] = { 20, 100, 52, 10004, 10004, 10008, 10008, 10012, 10012, 11004,
    11004, 11020, 11024, 68, 204, 304, 212, 72, 76, 80, 80, 80, 68 };


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(16384), 4096, 16);
    S0 = SP;	/* save base of stack */

    here = EP;	/* start assembling at 16 */
    start_ass();
    ass(O_BRANCHI); ilit(19);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)(M0 + 96);	/* start assembling at 96 */
    start_ass();
    ass(O_BRANCHI); ilit(-13);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)(M0 + 48);	/* start assembling at 48 */
    start_ass();
    ass(O_BRANCH); lit(10000);
    end_ass();

    here = (CELL *)(M0 + 10000);    /* start assembling at 10000 */
    start_ass();
    ass(O_ONE); ass(O_QBRANCHI); ilit(0);
    ass(O_ONE); ass(O_QBRANCH); lit(0); ass(O_ZERO); ass(O_QBRANCH); lit(11000);
    end_ass();

    here = (CELL *)(M0 + 11000);    /* start assembling at 11000 */
    start_ass();
    ass(O_ZERO); ass(O_QBRANCHI); ilit(3);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)(M0 + 11016);    /* start assembling at 11016 */
    start_ass();
    ass(O_LITERALI); ilit(64);
    ass(O_EXECUTE);
    end_ass();

    here = (CELL *)(M0 + 64);	/* start assembling at 64 */
    start_ass();
    ass(O_CALLI); ilit(33);
    ass(O_LITERALI); ilit(64);
    ass(O_LITERALI); ilit(20);
    ass(O_TUCK); ass(O_STORE); ass(O_FEXECUTE);
    end_ass();

    here = (CELL *)(M0 + 200);	/* start assembling at 200 */
    start_ass();
    ass(O_CALL); lit(300); ilit(0); /* pad out word with NEXT (00h) */
    ass(O_EXIT);
    end_ass();

    here = (CELL *)(M0 + 300);	/* start assembling at 300 */
    start_ass();
    ass(O_EXIT);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
#ifdef B_DEBUG
        printf("EP = %d; should be %d\n\n", val_EP(), correct[i]);
#endif
    	if (correct[i] != val_EP()) {
    	    printf("Error in BranchT: EP = %ld\n", val_EP());
    	    exit(1);
    	}
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
    }

    printf("BranchT ran OK\n");
    return 0;
}
