// Test that run works, and that the return value of the HALT instruction is
// correctly returned.
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "btests.h"


int main(void)
{
    int exception = 0;

    int i = init_beetle((CELL *)calloc(1024, 1), 256);
    if (i != 0) {
        printf("Error in run() tests: init_beetle with valid parameters failed\n");
        exit(1);
    }

    here = 52;
    start_ass();
    ass(O_LITERALI); ilit(37);
    ass(O_HALT);
    end_ass();

    NEXT;
    CELL ret = run();

    printf("Return value should be 37 and is %"PRId32"\n", ret);
    if (ret != 37) {
        printf("Error in run() tests: incorrect return value from run\n");
        exit(1);
    }

    printf("EP should now be 56\n");
    if (EP != 60) {
        printf("Error in run() tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(exception == 0);
    printf("run() tests ran OK\n");
    return 0;
}
