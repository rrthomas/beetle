// Test the logic operators. Also uses the NEXT and -ROT instructions. We
// only test the stack handling and basic correctness of the operators here,
// assuming that if the logic works in one case, it will work in all (if the
// C compiler doesn't implement it correctly, we're in trouble anyway!).
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = {
    "-16777216 8 255 8", "-16777216 8 65280",
    "65280 -16777216 8", "65280 16711680", "16776960", "33553920", "16776960",
    "-16776961", "-16776961 1", "-16776961 1 -1", "-16776961 -2", "-16776962"};


int main(void)
{
    int exception = 0;

    init(256);

    PUSH(0xff000000); PUSH(8); PUSH(0xff); PUSH(8);

    start_ass(R(EP));
    ass(O_LSHIFT); ass(O_NROT); ass(O_RSHIFT); ass(O_OR);
    ass(O_LSHIFT1); ass(O_RSHIFT1); ass(O_INVERT); ass(O_ONE);
    ass(O_MONE); ass(O_XOR); ass(O_AND);

    assert(single_step() == EXIT_SINGLE_STEP);   // load first instruction word

    for (size_t i = 0; i - i / 5 < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in logic tests: EP = %"PRIu32"\n", R(EP));
            exit(1);
        }
        assert(single_step() == EXIT_SINGLE_STEP);
        printf("I = %s\n", disass(R(I)));
    }

    assert(exception == 0);
    printf("Logic tests ran OK\n");
    return 0;
}
