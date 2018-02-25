/* EXCEPTST.C

    (c) Reuben Thomas 1995-2018

    Test the Beetle-generated exceptions and HALT codes.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


UCELL test[] = { 16, 20, 28, 40, 52, 56, 60, 64, 72, 76, 80, 84, 88, 92 };
CELL result[] = { -258, -9, 100, 0, -23, -23, -10, -9, -9, -23, -256, -257, -258, -259 };
UCELL bad[] = { -1, 28, 28, 28, 56, 60, 64, 16384, 76, 80, 84, 88, 88, 100 };
UCELL address[] = { -4, 16384, 0, 0, 5, 1, 0, 16384, -4, 1, 0, 0, -4, -4 };


int main(void)
{
    init_beetle((CELL *)malloc(16384), 4096, 16);
    S0 = SP;	/* save base of stack */

    here = EP;	/* start assembling at 16 */
    start_ass();
    ass(O_ZERO); ass(O_SPSTORE); ass(O_DUP); ass(O_NEXT00); /* test 1 */
    ass(O_LITERALI); ilit(MEMORY);  /* test 2 */
    ass(O_SPSTORE); ass(O_TOR); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_CELL); ass(O_SPSTORE); ass(O_DUP); ass(O_DROP);   /* test 3 */
    ass(O_LITERALI); ilit(100);	/* reset 'THROW, overwritten by the DUP above */
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_LITERALI); ilit(MEMORY);  /* test 4 */
    ass(O_MINUSCELL); ass(O_SPSTORE); ass(O_TOR); ass(O_ZERO);
    ass(O_HALT); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_ONE); ass(O_PLUSCELL); ass(O_SPSTORE); ass(O_NEXT00);	/* test 5 */
    ass(O_ONE); ass(O_EXECUTE);	ass(O_NEXT00); ass(O_NEXT00);	/* test 6 */
    ass(O_ONE); ass(O_ZERO); ass(O_SLASH); ass(O_NEXT00);   /* test 7 */
    ass(O_BRANCH); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);	/* test 8 */
    lit(MEMORY - CELL_W);
    ass(O_MCELL); ass(O_FETCH);	ass(O_NEXT00); ass(O_NEXT00);	/* test 9 */
    ass(O_ONE); ass(O_FETCH); ass(O_NEXT00); ass(O_NEXT00); /* test 10 */
    ass(0x60);	ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);	/* test 11 */
    ass(O_MCELL); ass(O_LIB); ass(O_NEXT00); ass(O_NEXT00);	/* test 12 */
    ass(O_ZERO); ass(O_SPSTORE); ass(O_DUP); ass(O_NEXT00);	/* test 13 */
    ass(O_ONE); ass(O_DUP); ass(O_ZERO); ass(O_STORE);	/* test 14 */
    ass(O_THROW);
    end_ass();

    here = 100;	/* start assembling at 100 */
    start_ass();
    ass(O_HALT);
    end_ass();

    *THROW = 100;   /* set address of exception handler */

    UCELL error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = S0;    /* reset stack pointer */

        EP = test[i];
        NEXT;   /* load first instruction word */
        CELL res = run();

        printf("Test %zu\n", i + 1);
        if (result[i] != res || (result[i] != 0 && bad[i] != BAD) ||
            ((result[i] <= -258 || result[i] == 9 || result[i] == -23) &&
             address[i] != NOT_ADDRESS)) {
             printf("Error in ExceptsT: test %zu failed; EP = %"PRIu32"\n", i + 1, EP);
             printf("Return code is %d; should be %d\n", res, result[i]);
             if (result[i] != 0)
                 printf("'BAD = %"PRIX32"; should be %"PRIX32"\n", BAD, bad[i]);
             if (result[i] <= -258 || result[i] == 9 || result[i] == -23)
                 printf("-ADDRESS = %"PRIX32"; should be %"PRIX32"\n", NOT_ADDRESS, address[i]);
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("ExceptsT ran OK\n");
    return error;
}
