// Test the arithmetic operators. Also uses the NEXT, SWAP, ROT,
// DROP, and (LITERAL)I instructions. Since unsigned arithmetic
// overflow behaviour is guaranteed by the ISO standard, we only test
// the stack handling and basic correctness of the operators here,
// assuming that if the arithmetic works in one case, it will work in
// all. Note that the correct stack values are not quite independent
// of the cell size (in CELL_W and str(CELL_W)); some stack pictures
// implicitly refer to it.
//
// (c) Reuben Thomas 1994-2018

#include "btests.h"


const char *correct[] = {
    "", "0", "0 1", "0 1 -1", "0 1 -1 " str(CELL_W),
    "0 1 -1 " str(CELL_W) " -" str(CELL_W), "0 1 " str(CELL_W) " -" str(CELL_W) " -1",
    "0 1 " str(CELL_W) " -5", "0 1 -1", "0 2", "0 3", "0 2", "2 0", "2 " str(CELL_W),
    "2 0", "2 0 -1", "2 0 -1 " str(CELL_W), "2 0 -" str(CELL_W), "2 -" str(CELL_W), "-2 -1",
    "2", "2 -1", "0", "1", str(CELL_W), "2", "", str(CELL_W), "-" str(CELL_W), str(CELL_W),
    str(CELL_W), str(CELL_W) " 1", str(CELL_W), str(CELL_W) " -" str(CELL_W), "-" str(CELL_W),
    "-" str(CELL_W) " 3", "-1 -1", "-1", "-1 -2", "1 1" };


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)calloc(1024, 1), 256);
    here = EP;

    start_ass();
    ass(O_ZERO); ass(O_ONE); ass(O_MONE); ass(O_CELL);
    ass(O_MCELL); ass(O_ROT); ass(O_PLUS); ass(O_PLUS);
    ass(O_MINUS); ass(O_PLUS1); ass(O_MINUS1); ass(O_SWAP);
    ass(O_PLUSCELL); ass(O_MINUSCELL); ass(O_MONE); ass(O_CELL);
    ass(O_STAR); ass(O_SWAPMINUS); ass(O_SLASHMOD); ass(O_SLASH);
    ass(O_MONE); ass(O_MOD); ass(O_PLUS1); ass(O_CELLS);
    ass(O_SLASH2); ass(O_DROP); ass(O_CELL); ass(O_NEGATE);
    ass(O_ABS); ass(O_ABS); ass(O_ONE); ass(O_MAX);
    ass(O_MCELL); ass(O_MIN); ass(O_LITERALI); ilit(3);
    ass(O_SSLASHREM); ass(O_DROP); ass(O_LITERALI); ilit(-2);
    ass(O_USLASHMOD); ass(O_NEXTFF);
    end_ass();

    NEXT;   // load first instruction word

    for (int i = 0; i <= instrs - instrs / 5; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        if (I == O_NEXT00)
            i--;
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("Arithmetic tests ran OK\n");
    return 0;
}
