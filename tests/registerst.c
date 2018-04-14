/* REGISTERST.C

    (c) Reuben Thomas 1994-2018

    Test the register instructions; except for those operating on RP and SP
    (see memoryt.c). Also uses NEXT.

*/


#include "btests.h"


#define SIZE 1024

const char *correct[] = {
    "", str(CELL_W), "", "-50327552", "", "4096", "", "-33550336", "",
    "4096", "", "168", "", "168", "", str(SIZE), "", "-1", "-1 -1",
};


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)malloc(SIZE), SIZE / CELL_W);
    here = EP;

    start_ass();
    ass(O_EPFETCH); ass(O_DROP);  ass(O_S0FETCH); ass(O_DROP);
    ass(O_HASHS); ass(O_DROP);    ass(O_R0FETCH); ass(O_DROP);
    ass(O_HASHR); ass(O_DROP);    ass(O_LITERAL); ass(O_THROWSTORE);
    lit(168); // 42 CELLS
    ass(O_THROWFETCH); ass(O_DROP);  ass(O_MEMORYFETCH); ass(O_DROP);
    ass(O_BADFETCH); ass(O_NOT_ADDRESSFETCH);
    end_ass();

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in RegistersT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("RegistersT ran OK\n");
    return 0;
}
