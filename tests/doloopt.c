/* DOLOOPT.C

    (c) Reuben Thomas 1994-2018

    Test the DO...LOOP support instructions. Also uses instructions with
    lower opcodes.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


const char *correct[] = { "0 1 2", "3 2 1 0", "1", "1 2 3 4", "1 1" };


int main(void)
{
    int i;

    init_beetle((CELL *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */

    start_ass();
    ass(O_LITERALI); ilit(3);
    ass(O_ZERO); ass(O_DO); ilit(0);	/* pad instruction word with NEXT */
    ass(O_RFETCH); ass(O_LOOP); lit(24); ilit(0);   /* pad instruction word with
                                                       NEXT */
    ass(O_ZERO); ass(O_LITERAL); lit(3); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_MONE); ass(O_PLOOPI); ilit(-1);
    ass(O_CELL); ass(O_ONE); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_CELL); ass(O_PLOOP); lit(48); ass(O_NEXT00);
    ass(O_ONE); ass(O_ONE); ass(O_DO); ass(O_NEXT00);
    ass(O_RFETCH); ass(O_LOOPI); ilit(-1);
    ass(O_ONE); ass(O_TOR); ass(O_CELL); ass(O_TOR);
    ass(O_MONE); ass(O_TOR); ass(O_J); ass(O_NEXT00);
    ass(O_DUP); ass(O_UNLOOP);
    end_ass();

    NEXT;   /* load first instruction word */

    while (val_EP() < 36) single_step();
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[0]);
    if (strcmp(correct[0], val_data_stack())) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }
    SP = S0;

    while (val_EP() < 48) single_step();
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[1]);
    if (strcmp(correct[1], val_data_stack())) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }
    SP = S0;

    while (val_EP() < 56) single_step();
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[2]);
    if (strcmp(correct[2], val_data_stack())) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }
    SP = S0;

    for (i = 0; i < 12; i++) single_step();
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[3]);
    if (strcmp(correct[3], val_data_stack())) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }
    SP = S0;

    EP = (CELL *)(64 + (BYTE *)M0);  NEXT;	/* start execution at 64 */
    while ((EP - M0) * CELL_W < 76) single_step();
    printf("3rd item on return stack is %"PRId32" (should be %"PRId32").\n", *(RP + 2), *SP);
    if (*(RP + 2) != *SP) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    single_step(); single_step();
    show_data_stack();
    printf("Correct stack: %s\n\n", correct[4]);
    if (strcmp(correct[4], val_data_stack())) {
        printf("Error in DoLoopT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    printf("DoLoopT ran OK\n");
    return 0;
}
