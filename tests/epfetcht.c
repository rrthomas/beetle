/* EPFETCHT.C

    (c) Reuben Thomas 1994-2018

    Also uses NEXT.

*/


#include "btests.h"


unsigned correct[] = { 4, 4, 4 };


int main(void)
{
    int exception = 0;

    init_beetle((CELL *)malloc(1024), 256);
    here = EP;

    start_ass();
    ass(O_EPFETCH);
    end_ass();

    NEXT;   /* load first instruction word */

    for (int i = 0; i <= instrs; i++) {
        printf("EP = %u; should be %u\n\n", EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in EpfetchT: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("EpfetchT ran OK\n");
    return 0;
}
