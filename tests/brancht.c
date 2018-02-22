/* BRANCHT.C

    (c) Reuben Thomas 1994-2018

    Test the branch instructions. Also uses other instructions with lower
    opcodes than the instructions tested (i.e. those already tested).
    See exceptst.c for address exception handling tests.
    The test program contains an infinite loop, but this is only executed
    once.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


int correct[] = { 20, 100, 52, 10004, 10004, 10008, 10008, 10012, 10012, 11004,
    11004, 11020, 11024, 68, 204, 304, 212, 72, 76, 80, 80, 80, 68 };


int main(void)
{
    int i;

    size_t size = 4096;
    init_beetle((CELL *)calloc(size, sizeof(CELL)), size, 16);
    S0 = M0 + SP / CELL_W;	/* save base of stack */

    here = EP;	/* start assembling at 16 */
    start_ass();
    ass(O_BRANCHI); ilit(19);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)((BYTE *)M0 + 96);	/* start assembling at 96 */
    start_ass();
    ass(O_BRANCHI); ilit(-13);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)((BYTE *)M0 + 48);	/* start assembling at 48 */
    start_ass();
    ass(O_BRANCH); lit(10000);
    end_ass();

    here = (CELL *)((BYTE *)M0 + 10000);    /* start assembling at 10000 */
    start_ass();
    ass(O_ONE); ass(O_QBRANCHI); ilit(0);
    ass(O_ONE); ass(O_QBRANCH); lit(0); ass(O_ZERO); ass(O_QBRANCH); lit(11000);
    end_ass();

    here = (CELL *)((BYTE *)M0 + 11000);    /* start assembling at 11000 */
    start_ass();
    ass(O_ZERO); ass(O_QBRANCHI); ilit(3);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = (CELL *)((BYTE *)M0 + 11016);    /* start assembling at 11016 */
    start_ass();
    ass(O_LITERALI); ilit(64);
    ass(O_EXECUTE);
    end_ass();

    here = (CELL *)((BYTE *)M0 + 64);	/* start assembling at 64 */
    start_ass();
    ass(O_CALLI); ilit(33);
    ass(O_LITERALI); ilit(64);
    ass(O_LITERALI); ilit(20);
    ass(O_TUCK); ass(O_STORE); ass(O_FEXECUTE);
    end_ass();

    here = (CELL *)((BYTE *)M0 + 200);	/* start assembling at 200 */
    start_ass();
    ass(O_CALL); lit(300); ilit(0); /* pad out word with NEXT (00h) */
    ass(O_EXIT);
    end_ass();

    here = (CELL *)((BYTE *)M0 + 300);	/* start assembling at 300 */
    start_ass();
    ass(O_EXIT);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
        printf("EP = %d; should be %d\n\n", val_EP(), correct[i]);
        if (correct[i] != val_EP()) {
            printf("Error in BranchT: EP = %"PRId32"d\n", val_EP());
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    printf("BranchT ran OK\n");
    return 0;
}
