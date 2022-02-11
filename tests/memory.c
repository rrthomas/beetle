// Test the memory operators. Also uses previously tested instructions.
// See exceptions.c for address exception handling tests.
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
    "", "16384", "16380", "16380 513", "16380 513 16380", "16380",
    "16380 16380", "16380 513", "16380",
    "16380 16380", "16380 1", "16381", "2", "2 16383", "", "16380", "33554945",
    "", "16128", "", "16384", "", "0", "", "0",
};


int main(void)
{
    init(4096);

    start_ass(R(EP));
    ass(O_MEMORYFETCH); ass(O_MINUSCELL); ass(O_LITERAL); lit(513);
    ass(O_OVER); ass(O_STORE); ass(O_DUP); ass(O_FETCH);
    ass(O_DROP); ass(O_DUP); ass(O_CFETCH); ass(O_PLUS);
    ass(O_CFETCH); ass(O_LITERAL); lit(16383); ass(O_CSTORE); ass(O_LITERAL);
        lit(16380);
    ass(O_FETCH); ass(O_DROP); ass(O_SPFETCH); ass(O_SPSTORE);
    ass(O_RPFETCH); ass(O_DROP); ass(O_ZERO); ass(O_RPSTORE);
    ass(O_RPFETCH);

    assert(single_step() == EXIT_SINGLE_STEP);   // load first instruction word

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in memory tests: EP = %"PRIu32"\n", R(EP));
            exit(1);
        }
        assert(single_step() == EXIT_SINGLE_STEP);
        printf("I = %s\n", disass(R(I)));
    }

    printf("Memory tests ran OK\n");
    return 0;
}
