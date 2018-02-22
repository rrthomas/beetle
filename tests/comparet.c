/* COMPARET.C

    (c) Reuben Thomas 1994-2018

    Test the comparison operators. Also uses the NEXT instruction. We
    only test simple cases here, assuming that the C compiler's comparison
    routines will work for other cases.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


CELL correct[] = { 0, -1, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0,
    -1, 0, 0, -1, 0, 0, -1, 0, 0, -1};


static void stack1(void)
{
    int exception = 0; // FIXME

    SP = S0;	/* empty the stack */

    PUSH(-4); PUSH(3);
    PUSH(2); PUSH(2);
    PUSH(1); PUSH(3);
    PUSH(3); PUSH(1);
}

static void stack2(void)
{
    int exception = 0; // FIXME

    SP = S0;	/* empty the stack */

    PUSH(1); PUSH(-1);
    PUSH(237); PUSH(237);
}

static void stack3(void)
{
    int exception = 0; // FIXME

    SP = S0;	/* empty the stack */

    PUSH(-1); PUSH(0); PUSH(237);
}

static void step(int start, int end)
{
    int exception = 0; // FIXME
    CELL temp; // FIXME
    int i;

    for (i = start; i <= end; i++) {
        single_step();
        printf("I = %s\n", disass(I));
        if (I != O_NEXT00) {
            printf("Result: %d; correct result: %d\n\n", LOAD_CELL(SP),
                correct[i - i / 5]);
            if (correct[i - i / 5] != LOAD_CELL(SP)) {
                printf("Error in CompareT: EP = %"PRIu32"\n", EP);
                exit(1);
            }
            (void)POP;	/* drop result of comparison */
        }
        else
          putchar('\n');
    }
}

int main(void)
{
    int exception = 0; // FIXME
    CELL temp; // FIXME

    init_beetle((CELL *)malloc(1024), 256, 16);
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
    SP = S0;  PUSH(237); PUSH(0);	/* set up the stack with two values */
    step(22, 24);   /* do the 0= tests */
    stack1();       /* set up the stack with four standard pairs to compare */
    step(25, 29);   /* do the U< tests */
    stack1();
    step(30, 34);   /* do the U> tests */

    printf("CompareT ran OK\n");
    return 0;
}
