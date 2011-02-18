/* LITERALT.C

    (c) Reuben Thomas 1994-2011

    Test the literal instructions. Also uses the NEXT instruction.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


char *correct[] = { "", "-257", "-257 12345678", "-257 12345678 -2" };


int main(void)
{
    int i;

    init_beetle((BYTE *)calloc(1024, 1), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_LITERAL); lit(-257); ass(O_LITERAL); lit(12345678);
    ass(O_LITERALI); ilit(-2);
    end_ass();
    instrs++;	/* instrs is out by one when an immediate literal is the last
                   thing assembled */

    NEXT;   /* load first instruction word */

    for (i = 0; i <= instrs; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in LiteralT: EP = %"PRId32"\n", val_EP());
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }
    printf("LiteralT ran OK\n");
    return 0;
}
