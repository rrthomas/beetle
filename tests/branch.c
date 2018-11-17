// Test the branch instructions. Also uses other instructions with lower
// opcodes than the instructions tested (i.e. those already tested).
// See exceptions.c for address exception handling tests.
// The test program contains an infinite loop, but this is only executed
// once.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "tests.h"


unsigned correct[] = { 4, 100, 52, 10004, 10004, 10008, 10008, 10012, 10012, 11004,
                       11004, 11020, 11024, 68, 204, 304, 212, 72, 76, 80, 80, 80, 68 };


int main(void)
{
    int exception = 0;

    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(EP);
    ass(O_BRANCHI); ilit(23);

    start_ass(96);
    ass(O_BRANCHI); ilit(-13);

    start_ass(48);
    ass(O_BRANCH); lit(10000);

    start_ass(10000);
    ass(O_ONE); ass(O_QBRANCHI); ilit(0);
    ass(O_ONE); ass(O_QBRANCH); lit(0); ass(O_ZERO); ass(O_QBRANCH); lit(11000);

    start_ass(11000);
    ass(O_ZERO); ass(O_QBRANCHI); ilit(3);

    start_ass(11016);
    ass(O_LITERALI); ilit(64);
    ass(O_EXECUTE);

    start_ass(64);
    ass(O_CALLI); ilit(33);
    ass(O_LITERALI); ilit(64);
    ass(O_LITERALI); ilit(20);
    ass(O_TUCK); ass(O_STORE); ass(O_FEXECUTE);

    start_ass(200);
    ass(O_CALL); lit(300); ilit(0);
    ass(O_EXIT);

    start_ass(300);
    ass(O_EXIT);

    assert(single_step() == -259);   // load first instruction word

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction %zu: EP = %u; should be %u\n\n", i, EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in branch tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("Branch tests ran OK\n");
    return 0;
}
