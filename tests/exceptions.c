// Test the VM-generated exceptions and HALT codes.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


UCELL test[] = { 0, 8, 16, 24, 36, 40, 44, 48, 56, 64, 68, 72 };
CELL result[] = { -257, -257, 42, 0, -23, -23, -10, -9, -9, -23, -256, -258 };
UCELL bad[] = { -1, -1, -1, 28, 40, 44, 48, 16388, 64, 68, 72, 80 };
UCELL address[] = { -16, 16384, 0, 0, 5, 1, 0, 16384, -20, 1, 0, 1 };


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(0);
    // test 1: DUP into non-existent memory
    ass(O_LITERAL); lit(0xfffffff0);
    ass(O_SPSTORE); ass(O_DUP); ass(O_NEXT00);
    // test 2: set SP to MEMORY, then try to pop (>R) the stack
    ass(O_LITERALI); ilit(MEMORY);
    ass(O_SPSTORE); ass(O_TOR); ass(O_NEXT00); ass(O_NEXT00);
    // test 3: test arbitrary throw code
    ass(O_LITERALI); ilit(42);
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    // test 4: test SP can point to just after memory
    ass(O_LITERALI); ilit(MEMORY);
    ass(O_MINUSCELL); ass(O_SPSTORE); ass(O_TOR); ass(O_ZERO);
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    // test 5: test setting SP to unaligned address
    ass(O_ONE); ass(O_PLUSCELL); ass(O_SPSTORE); ass(O_NEXT00);
    // test 6: test EXECUTE on unaligned address
    ass(O_ONE); ass(O_EXECUTE);	ass(O_NEXT00); ass(O_NEXT00);
    // test 7: test division by zero
    ass(O_ONE); ass(O_ZERO); ass(O_SLASH); ass(O_NEXT00);
    // test 8: allow execution to run off the end of memory
    ass(O_BRANCH); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    lit(MEMORY - CELL_W);
    // test 9: fetch from an invalid address
    ass(O_LITERAL); lit(0xffffffec);
    ass(O_FETCH); ass(O_NEXT00); ass(O_NEXT00);
    // test 10: fetch from an unaligned address
    ass(O_ONE); ass(O_FETCH); ass(O_NEXT00); ass(O_NEXT00);
    // test 11: test invalid opcode
    ass(O_UNDEFINED); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    // test 12: test invalid 'THROW contents
    ass(O_LITERAL); lit(0xffffffec);
    ass(O_DUP); ass(O_THROWSTORE); ass(O_THROW);

    start_ass(200);
    ass(O_HALT);

    THROW = 200;   // set address of exception handler

    UCELL error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = S0;    // reset stack pointer

        printf("Test %zu\n", i + 1);
        EP = test[i];
        assert(single_step() == -259);   // load first instruction word
        CELL res = run();

        if (result[i] != res || (result[i] != 0 && bad[i] != BAD) ||
            ((result[i] <= -257 || result[i] == -9 || result[i] == -23) &&
             address[i] != NOT_ADDRESS)) {
             printf("Error in exceptions tests: test %zu failed; EP = %"PRIu32"\n", i + 1, EP);
             printf("Return code is %d; should be %d\n", res, result[i]);
             if (result[i] != 0)
                 printf("'BAD = %"PRIX32"; should be %"PRIX32"\n", BAD, bad[i]);
             if (result[i] <= -257 || result[i] == -9 || result[i] == -23)
                 printf("-ADDRESS = %"PRIX32"; should be %"PRIX32"\n", NOT_ADDRESS, address[i]);
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("Exceptions tests ran OK\n");
    return error;
}
