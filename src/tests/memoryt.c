/* MEMORYT.C

    Vrsn  Date   Comment
    ----|-------|--------------------------------------------------------------
    0.00 23nov94 Test @, !, C@, C!, +!, SP@, SP!, RP@ and RP!.
    0.01 28nov94 Changed reference to b_mem to one to M0.
    0.02 30nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information. Modified to give a return value
    	    	 from main.
    0.03 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.
    0.04 28feb95 Removed printf format error.

    Reuben Thomas


    Test the memory operators. Also uses previously tested instructions.
    Doesn't test address exception handling, as this is not yet supported.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


char *correct[] = { "", QCELL_W, "16384", "16380", "16380 513",
    "16380 513 16380", "16380", "16380 16380", "16380 513", "16380",
    "16380 16380", "16380 1", "16381", "2", "2 16383", "", "16380", "33554945",
    "", "16128", "", "16384", "", "0", "", "0" };


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(16384), 4096, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_CELL); ass(O_FETCH); ass(O_MINUSCELL); ass(O_LITERAL); lit(513);
    ass(O_OVER); ass(O_STORE); ass(O_DUP); ass(O_FETCH);
    ass(O_DROP); ass(O_DUP); ass(O_CFETCH); ass(O_PLUS);
    ass(O_CFETCH); ass(O_LITERAL); lit(16383); ass(O_CSTORE); ass(O_LITERAL);
    	lit(16380);
    ass(O_FETCH); ass(O_DROP); ass(O_SPFETCH); ass(O_SPSTORE);
    ass(O_RPFETCH); ass(O_DROP); ass(O_ZERO); ass(O_RPSTORE);
    ass(O_RPFETCH);
    end_ass();

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
#ifdef B_DEBUG
        show_data_stack();  printf("Correct stack: %s\n\n", correct[i - i / 5]);
#endif
        if (strcmp(correct[i - i / 5], val_data_stack())) {
    	    printf("Error in MemoryT: EP = %ld\n", val_EP());
    	    exit(1);
    	}
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
    }

    printf("MemoryT ran OK\n");
    return 0;
}
