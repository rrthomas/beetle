// Test that single_step works.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    init(256);

    for (int i = 0; i < 10; i++) {
        printf("EP = %u\n", R(EP));
        assert(single_step() == EXIT_SINGLE_STEP);
    }

    printf("EP should now be 40\n");
    if (R(EP) != 40) {
        printf("Error in single_step() tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    return 0;
}
