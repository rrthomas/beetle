/* LITERALT.C

    (c) Reuben Thomas 1994-2018

    Test the literal instructions. Also uses the NEXT instruction.

*/


#include "btests.h"


const char *correct[] = { "", "-257", "-257 12345678", "-257 12345678 -2" };


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)calloc(1024, 1), 256);
    here = EP;

    start_ass();
    ass(O_LITERAL); lit(-257); ass(O_LITERAL); lit(12345678);
    ass(O_LITERALI); ilit(-2);
    end_ass();
    instrs++;	/* instrs is out by one when an immediate literal is the last
                   thing assembled */

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in LiteralT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("LiteralT ran OK\n");
    return 0;
}
