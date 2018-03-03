/* MEMORYT.C

    (c) Reuben Thomas 1994-2018

    Test the memory operators. Also uses previously tested instructions.
    See exceptst.c for address exception handling tests.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


const char *correct[] = {
    "", QCELL_W, "16384", "16380", "16380 513",
    "16380 513 16380", "16380", "16380 16380", "16380 513", "16380",
    "16380 16380", "16380 1", "16381", "2", "2 16383", "", "16380", "33554945",
    "", "16128", "", "16384", "", "0", "", "0", "", "-2147483648", "67305985",
    "", "-2147483643", "2", "", "1", "1 -2147483647", "", "-2147483647", "1", "",
};


int main(void)
{
    /* Data for himem tests */
    char *onetwothreefour = strdup("\x01\x02\x03\x04");
    char *item[] = {onetwothreefour, strdup("\x01"), strdup("\x02\x03"), strdup("basilisk")};
    unsigned nitems = sizeof(item) / sizeof(item[0]);

    init_beetle((CELL *)malloc(16384), 4096);
    for (unsigned i = 0; i < nitems; i++) {
        printf("Himem item %u allocated at address %"PRIX32"\n",
               i, himem_allot(item[i], strlen(item[i])));
        if (i == 2)
            himem_align();
    }
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
    ass(O_RPFETCH); ass(O_DROP);
    ass(O_LITERAL); lit(0x80000000); ass(O_FETCH); ass(O_DROP);
    ass(O_LITERAL); lit(0x80000005); ass(O_CFETCH); ass(O_DROP);
    ass(O_ONE); ass(O_LITERAL); lit(0x80000001); ass(O_CSTORE);
    ass(O_LITERAL); lit(0x80000001); ass(O_CFETCH); ass(O_DROP);
    end_ass();

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in MemoryT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    printf("MemoryT ran OK\n");
    return 0;
}
