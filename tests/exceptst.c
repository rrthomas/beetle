/* EXCEPTST.C

    (c) Reuben Thomas 1995-2018

    Test the Beetle-generated exceptions and HALT codes.

*/


#include "btests.h"


UCELL test[] = { 0, 8, 16, 28, 40, 44, 48, 52, 60, 68, 72, 76, 80 };
CELL result[] = { -258, -9, 200, 0, -23, -23, -10, -9, -9, -23, -256, -257, -259 };
UCELL bad[] = { -1, 16, 16, 32, 44, 48, 52, 16388, 68, 72, 76, 80, 88 };
UCELL address[] = { -20, 16384, 0, 0, 5, 1, 0, 16384, -20, 1, 0, 0, 1 };


int main(void)
{
    int exception = 0;

    size_t size = 4096;
    init_beetle((CELL *)calloc(size, CELL_W), size);
    S0 = SP;	/* save base of stack */

    here = EP;	/* start assembling at 0 */
    start_ass();
    // test 1: DUP into non-existent memory
    ass(O_LITERAL); lit(0xfffffff0);
    ass(O_SPSTORE); ass(O_DUP); ass(O_NEXT00);
    // test 2: set SP to MEMORY, then try to pop (>R) the stack
    ass(O_LITERALI); ilit(MEMORY);
    ass(O_SPSTORE); ass(O_TOR); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_CELL); ass(O_SPSTORE); ass(O_DUP); ass(O_DROP);   /* test 3 */
    ass(O_LITERALI); ilit(200);	/* reset 'THROW, overwritten by the DUP above */
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_LITERALI); ilit(MEMORY);  /* test 4 */
    ass(O_MINUSCELL); ass(O_SPSTORE); ass(O_TOR); ass(O_ZERO);
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_ONE); ass(O_PLUSCELL); ass(O_SPSTORE); ass(O_NEXT00);	/* test 5 */
    ass(O_ONE); ass(O_EXECUTE);	ass(O_NEXT00); ass(O_NEXT00);	/* test 6 */
    ass(O_ONE); ass(O_ZERO); ass(O_SLASH); ass(O_NEXT00);   /* test 7 */
    // test 8: allow execution to run off the end of a memory area
    ass(O_BRANCH); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    lit(MEMORY - CELL_W);
    // test 9: fetch from an invalid address
    ass(O_LITERAL); lit(0xffffffec);
    ass(O_FETCH); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_ONE); ass(O_FETCH); ass(O_NEXT00); ass(O_NEXT00); /* test 10 */
    ass(0x60);	ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);	/* test 11 */
    ass(O_MCELL); ass(O_LIB); ass(O_NEXT00); ass(O_NEXT00);	/* test 12 */
    // test 13: test invalid 'THROW contents
    ass(O_ONE); ass(O_DUP); ass(O_MCELL); ass(O_STORE);
    ass(O_THROW);
    end_ass();

    here = 200;	/* start assembling at 200 */
    start_ass();
    ass(O_HALT);
    end_ass();

    THROW = 200;   /* set address of exception handler */

    UCELL error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = S0;    /* reset stack pointer */

        printf("Test %zu\n", i + 1);
        EP = test[i];
        NEXT;   /* load first instruction word */
        CELL res = run();

        if (result[i] != res || (result[i] != 0 && bad[i] != BAD) ||
            ((result[i] <= -258 || result[i] == -9 || result[i] == -23) &&
             address[i] != NOT_ADDRESS)) {
             printf("Error in ExceptsT: test %zu failed; EP = %"PRIu32"\n", i + 1, EP);
             printf("Return code is %d; should be %d\n", res, result[i]);
             if (result[i] != 0)
                 printf("'BAD = %"PRIX32"; should be %"PRIX32"\n", BAD, bad[i]);
             if (result[i] <= -258 || result[i] == -9 || result[i] == -23)
                 printf("-ADDRESS = %"PRIX32"; should be %"PRIX32"\n", NOT_ADDRESS, address[i]);
             error++;
        }
        putchar('\n');
    }

    assert(exception == 0);
    if (error == 0)
        printf("ExceptsT ran OK\n");
    return error;
}
