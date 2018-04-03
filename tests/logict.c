/* LOGICT.C

    (c) Reuben Thomas 1994-2018

    Test the logic operators. Also uses the NEXT and -ROT instructions. We
    only test the stack handling and basic correctness of the operators here,
    assuming that if the logic works in one case, it will work in all (if the
    C compiler doesn't implement it correctly, we're in trouble anyway!).

*/


#include "btests.h"


const char *correct[] = {
    "-16777216 8 255 8", "-16777216 8 65280",
    "65280 -16777216 8", "65280 16711680", "16776960", "33553920", "16776960",
    "-16776961", "-16776961 1", "-16776961 1 -1", "-16776961 -2", "-16776962"};


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)malloc(1024), 256);
    here = EP;

    PUSH(0xFF000000); PUSH(8); PUSH(0xFF); PUSH(8);

    start_ass();
    ass(O_LSHIFT); ass(O_NROT); ass(O_RSHIFT); ass(O_OR);
    ass(O_LSHIFT1); ass(O_RSHIFT1); ass(O_INVERT); ass(O_ONE);
    ass(O_MONE); ass(O_XOR); ass(O_AND);
    end_ass();

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in LogicT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("LogicT ran OK\n");
    return 0;
}
