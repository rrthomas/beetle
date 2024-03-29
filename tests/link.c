// Test the LINK instruction.
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int exception = 0;
CELL temp;


static void test(void)
{
    PUSH(37);
}

int main(void)
{
    init(4096);

    start_ass(R(EP));
    plit(test); ass(O_LINK); ass(O_ZERO); ass(O_HALT);

    assert(single_step() == EXIT_SINGLE_STEP);   // load first instruction word
    CELL res = run();
    if (res != 0) {
        printf("Error in LINK tests: test aborted with return code %"PRId32"\n", res);
        exit(1);
    }

    printf("Top of stack is %d; should be %d\n", LOAD_CELL(R(SP)), 37);
    show_data_stack();
    if (LOAD_CELL(R(SP)) != 37) {
        printf("Error in LINK tests: incorrect value on top of stack\n");
        exit(1);
    }

    assert(exception == 0);
    printf("LINK tests ran OK\n");
    return 0;
}
