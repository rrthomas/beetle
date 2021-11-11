// Test the comparison operators. Also uses the NEXT instruction. We
// only test simple cases here, assuming that the C compiler's comparison
// routines will work for other cases.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int exception = 0;
CELL temp;

CELL correct[] = { 0, -1, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0,
    -1, 0, 0, -1, 0, 0, -1, 0, 0, -1};


static void stack1(void)
{
    R(SP) = R(S0);	// empty the stack

    PUSH(-4); PUSH(3);
    PUSH(2); PUSH(2);
    PUSH(1); PUSH(3);
    PUSH(3); PUSH(1);
}

static void stack2(void)
{
    R(SP) = R(S0);	// empty the stack

    PUSH(1); PUSH(-1);
    PUSH(237); PUSH(237);
}

static void stack3(void)
{
    R(SP) = R(S0);	// empty the stack

    PUSH(-1); PUSH(0); PUSH(237);
}

static void step(unsigned start, unsigned end)
{
    if (end > start)
        for (unsigned i = start; i < end; i++) {
            single_step();
            printf("I = %s\n", disass(R(I)));
            if (R(I) != O_NEXT00) {
                printf("Result: %d; correct result: %d\n\n", LOAD_CELL(R(SP)),
                       correct[i - i / 5]);
                if (correct[i - i / 5] != LOAD_CELL(R(SP))) {
                    printf("Error in comparison tests: EP = %"PRIu32"\n", R(EP));
                    exit(1);
                }
                (void)POP;	// drop result of comparison
            }
            else
                putchar('\n');
        }
}

int main(void)
{
    init(256);

    start_ass(R(EP));
    ass(O_LESS); ass(O_LESS); ass(O_LESS); ass(O_LESS);
    ass(O_GREATER); ass(O_GREATER); ass(O_GREATER); ass(O_GREATER);
    ass(O_EQUAL); ass(O_EQUAL); ass(O_NEQUAL); ass(O_NEQUAL);
    ass(O_LESS0); ass(O_LESS0); ass(O_LESS0); ass(O_GREATER0);
    ass(O_GREATER0); ass(O_GREATER0); ass(O_EQUAL0); ass(O_EQUAL0);
    ass(O_ULESS); ass(O_ULESS); ass(O_ULESS); ass(O_ULESS);
    ass(O_UGREATER); ass(O_UGREATER); ass(O_UGREATER); ass(O_UGREATER);

    assert(single_step() == -259);   // load first instruction word

    stack1();       // set up the stack with four standard pairs to compare
    step(0, 5);     // do the < tests
    stack1();
    step(5, 10);     // do the > tests
    stack2();       // set up the stack with two standard pairs to compare
    step(10, 12);   // do the = tests
    stack2();
    step(12, 15);   // do the <> tests
    stack3();       // set up the stack with three standard values
    step(15, 18);   // do the 0< tests
    stack3();
    step(18, 22);   // do the 0> tests
    R(SP) = R(S0);  PUSH(237); PUSH(0);	// set up the stack with two values
    step(22, 25);   // do the 0= tests
    stack1();       // set up the stack with four standard pairs to compare
    step(25, 30);   // do the U< tests
    stack1();
    step(30, 35);   // do the U> tests

    assert(exception == 0);
    printf("Comparison tests ran OK\n");
    return 0;
}
