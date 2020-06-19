// Test the register instructions, except for those operating on RP and SP
// (see memory.c). Also uses NEXT.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


#define SIZE 1024

const char *correct[] = {
    "", str(CELL_W), "", "768", "", str(SIZE), "",
    "168", "", "168", "", str(SIZE), "", "-1", "-1 -1",
};


int main(void)
{
    init((CELL *)malloc(SIZE), SIZE / CELL_W);

    start_ass(EP);
    ass(O_EPFETCH); ass(O_DROP);  ass(O_S0FETCH); ass(O_DROP);
    ass(O_R0FETCH); ass(O_DROP);  ass(O_LITERAL); ass(O_THROWSTORE);
    lit(168); // 42 CELLS
    ass(O_THROWFETCH); ass(O_DROP);  ass(O_MEMORYFETCH); ass(O_DROP);
    ass(O_BADFETCH); ass(O_NOT_ADDRESSFETCH);

    assert(single_step() == -259);   // load first instruction word

    for (size_t i = 0; i + i / 5 < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in registers tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        assert(single_step() == -259);
        printf("I = %s\n", disass(I));
    }

    printf("Registers tests ran OK\n");
    return 0;
}
