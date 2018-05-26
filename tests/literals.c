// Test the literal instructions. Also uses the NEXT instruction.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "btests.h"


const char *correct[] = { "", "-257", "-257 12345678", "-257 12345678 -2" };


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)calloc(1024, 1), 256);

    start_ass(EP);
    ass(O_LITERAL); lit(-257); ass(O_LITERAL); lit(12345678);
    ass(O_LITERALI); ilit(-2);

    NEXT;   // load first instruction word

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in literals tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("Literals tests ran OK\n");
    return 0;
}
