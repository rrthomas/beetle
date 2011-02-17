/* COMPARET.C

    (c) Reuben Thomas 1994-1995

    Test the comparison operators. Also uses the NEXT instruction. We
    only test simple cases here, assuming that the C compiler's comparison
    routines will work for other cases.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


CELL correct[] = { 0, -1, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0,
    -1, 0, 0, -1, 0, 0, -1, 0, 0, -1};


void stack1(void)
{
    SP = S0;	/* empty the stack */

    *--SP = -4; *--SP = 3; *--SP = 2; *--SP = 2; *--SP = 1; *--SP = 3;
    *--SP = 3; *--SP = 1;
}

void stack2(void)
{
    SP = S0;	/* empty the stack */
    *--SP = 1; *--SP = -1; *--SP = 237; *--SP = 237;
}

void stack3(void)
{
    SP = S0;	/* empty the stack */

    *--SP = -1; *--SP = 0; *--SP = 237;
}

void step(int start, int end)
{
    int i;

    for (i = start; i <= end; i++) {
        single_step();
#ifdef B_DEBUG
        printf("I = %s\n", disass(I));
#endif
        if (I != O_NEXT00) {
#ifdef B_DEBUG
            printf("Result: %d; correct result: %d\n\n", *SP,
                correct[i - i / 5]);
#endif
            if (correct[i - i / 5] != *SP) {
                printf("Error in CompareT: EP = %ld\n", val_EP());
                exit(1);
            }
            SP++;	/* drop result of comparison */
        }
#ifdef B_DEBUG
        else putchar('\n');
#endif
    }
}

int main(void)
{
    init_beetle((BYTE *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_LESS); ass(O_LESS); ass(O_LESS); ass(O_LESS);
    ass(O_GREATER); ass(O_GREATER); ass(O_GREATER); ass(O_GREATER);
    ass(O_EQUAL); ass(O_EQUAL); ass(O_NEQUAL); ass(O_NEQUAL);
    ass(O_LESS0); ass(O_LESS0); ass(O_LESS0); ass(O_GREATER0);
    ass(O_GREATER0); ass(O_GREATER0); ass(O_EQUAL0); ass(O_EQUAL0);
    ass(O_ULESS); ass(O_ULESS); ass(O_ULESS); ass(O_ULESS);
    ass(O_UGREATER); ass(O_UGREATER); ass(O_UGREATER); ass(O_UGREATER);
    end_ass();

    NEXT;   /* load first instruction word */

    stack1();       /* set up the stack with four standard pairs to compare */
    step(0, 4);     /* do the < tests */
    stack1();
    step(5, 9);     /* do the > tests */
    stack2();       /* set up the stack with two standard pairs to compare */
    step(10, 11);   /* do the = tests */
    stack2();
    step(12, 14);   /* do the <> tests */
    stack3();       /* set up the stack with three standard values */
    step(15, 17);   /* do the 0< tests */
    stack3();
    step(18, 21);   /* do the 0> tests */
    SP = S0;  *--SP = 237; *--SP = 0;	/* set up the stack with two values */
    step(22, 24);   /* do the 0= tests */
    stack1();       /* set up the stack with four standard pairs to compare */
    step(25, 29);   /* do the U< tests */
    stack1();
    step(30, 34);   /* do the U> tests */

    printf("CompareT ran OK\n");
    return 0;
}
