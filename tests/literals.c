// Test the literal instructions. Also uses the NEXT instruction.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = { "", "-257", "-257 12345678", "-257 12345678 -2" };


int main(void)
{
    init(256);

    start_ass(R(EP));
    ass(O_LITERAL); lit(-257); ass(O_LITERAL); lit(12345678);
    ass(O_LITERALI); ilit(-2);

    assert(single_step() == EXIT_SINGLE_STEP);   // load first instruction word

    for (size_t i = 0; i - i / 5 < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in literals tests: EP = %"PRIu32"\n", R(EP));
            exit(1);
        }
        assert(single_step() == EXIT_SINGLE_STEP);
        printf("I = %s\n", disass(R(I)));
    }

    printf("Literals tests ran OK\n");
    return 0;
}
