// Test that run works, and that the return value of the HALT instruction is
// correctly returned.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    int i = init(256);
    if (i != 0) {
        printf("Error in run() tests: init with valid parameters failed\n");
        exit(1);
    }

    start_ass(52);
    ass(O_LITERALI); ilit(37);
    ass(O_HALT);

    assert(single_step() == EXIT_SINGLE_STEP);
    CELL ret = run();

    printf("Return value should be 37 and is %"PRId32"\n", ret);
    if (ret != 37) {
        printf("Error in run() tests: incorrect return value from run\n");
        exit(1);
    }

    printf("EP should now be 56\n");
    if (R(EP) != 60) {
        printf("Error in run() tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }

    printf("run() tests ran OK\n");
    return 0;
}
