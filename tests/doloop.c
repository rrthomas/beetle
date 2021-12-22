// Test the DO...LOOP support instructions. Also uses instructions with
// lower opcodes.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = { "0 1 2", "3 2 1 0", "1", "1 2 3 4", "1 1" };


int main(void)
{
    int exception = 0;
    CELL temp = 0;

    init(256);

    start_ass(R(EP));
    // Address 0: 3 0 DO  R>  LOOP
    ass(O_LITERALI); ilit(3);
    ass(O_ZERO); ass(O_DO); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_LOOP); lit(8); ass(O_NEXT00); ass(O_NEXT00);
    // Address 16: 0 3 DO  R>  -1 +LOOP
    ass(O_ZERO); ass(O_LITERAL); lit(3); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_MONE); ass(O_PLOOPI); ilit(-1);
    // Address 32: CELL 1 DO  R>  CELL +LOOP
    ass(O_CELL); ass(O_ONE); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_CELL); ass(O_PLOOP); lit(32); ass(O_NEXT00);
    // Address 40: 1 1 DO  R>  LOOP  (infinite loop!)
    ass(O_ONE); ass(O_ONE); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_LOOPI); ilit(-1);
    // Address 48: 1 >R CELL >R -1 >R J DUP UNLOOP
    ass(O_ONE); ass(O_TOR); ass(O_CELL); ass(O_TOR);
    ass(O_MONE); ass(O_TOR); ass(O_J); ass(O_NEXT00);
    ass(O_DUP); ass(O_UNLOOP);

    assert(single_step() == EXIT_SINGLE_STEP);

    while (R(EP) < 20) assert(single_step() == EXIT_SINGLE_STEP);
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[0]);
    if (strcmp(correct[0], val_data_stack())) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }
    R(SP) = R(S0);

    while (R(EP) < 32) assert(single_step() == EXIT_SINGLE_STEP);
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[1]);
    if (strcmp(correct[1], val_data_stack())) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }
    R(SP) = R(S0);

    while (R(EP) < 40) assert(single_step() == EXIT_SINGLE_STEP);
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[2]);
    if (strcmp(correct[2], val_data_stack())) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }
    R(SP) = R(S0);

    for (int i = 0; i < 12; i++) assert(single_step() == EXIT_SINGLE_STEP);
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[3]);
    if (strcmp(correct[3], val_data_stack())) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }
    R(SP) = R(S0);

    assert(R(EP) == 48);
    R(A) = 0; R(I) = 0; // Exit infinite loop
    while (R(EP) < 60) assert(single_step() == EXIT_SINGLE_STEP);
    CELL ret3 = LOAD_CELL(R(RP) - 2 * CELL_W * STACK_DIRECTION);
    printf("3rd item on return stack is %"PRId32" (should be %"PRId32")\n", ret3, LOAD_CELL(R(SP)));
    if (ret3 != LOAD_CELL(R(SP))) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }

    assert(single_step() == EXIT_SINGLE_STEP);
    assert(single_step() == EXIT_SINGLE_STEP);
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[4]);
    if (strcmp(correct[4], val_data_stack())) {
        printf("Error in DO...LOOP tests: EP = %"PRIu32"\n", R(EP));
        exit(1);
    }

    assert(exception == 0);
    printf("DO...LOOP tests ran OK\n");
    return 0;
}
