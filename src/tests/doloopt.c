/* DOLOOPT.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 26nov94 Test (DO), (LOOP), (LOOP)I.
    0.01 27nov94 Added code to test (+LOOP) and (+LOOP)I.
    0.02 28nov94 Added code to test J, and used val_EP from debug.c (that's
    	    	 what it's for!).
    0.03 30nov94 Modified so that testing is automatic, and can run with or
    	    	 without debugging information. Modified to give a return value
    	    	 from main.
    0.04 13jan95 Added code to test UNLOOP.
    0.05 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.
    0.06 28feb95 Removed printf format errors.

    Reuben Thomas


    Test the DO...LOOP support instructions. Also uses instructions with
    lower opcodes.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


char *correct[] = { "0 1 2", "3 2 1 0", "1", "1 2 3 4", "1 1" };


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(1024), 256, 16);
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
#ifdef B_DEBUG
    show_data_stack();  printf("Correct stack: %s\n\n", correct[0]);
#endif
    if (strcmp(correct[0], val_data_stack())) {
    	printf("Error in DoLoopT: EP = %ld\n", val_EP());
    	exit(1);
    }
    SP = S0;

    while (val_EP() < 48) single_step();
#ifdef B_DEBUG
    show_data_stack();  printf("Correct stack: %s\n\n", correct[1]);
#endif
    if (strcmp(correct[1], val_data_stack())) {
    	printf("Error in DoLoopT: EP = %ld\n", val_EP());
    	exit(1);
    }
    SP = S0;

    while (val_EP() < 56) single_step();
#ifdef B_DEBUG
    show_data_stack();  printf("Correct stack: %s\n\n", correct[2]);
#endif
    if (strcmp(correct[2], val_data_stack())) {
    	printf("Error in DoLoopT: EP = %ld\n", val_EP());
    	exit(1);
    }
    SP = S0;

    for (i = 0; i < 12; i++) single_step();
#ifdef B_DEBUG
    show_data_stack();  printf("Correct stack: %s\n\n", correct[3]);
#endif
    if (strcmp(correct[3], val_data_stack())) {
    	printf("Error in DoLoopT: EP = %ld\n", val_EP());
    	exit(1);
    }
    SP = S0;

    EP = (CELL *)(64 + M0);  NEXT;	/* start execution at 64 */
    while (((BYTE *)EP - M0) < 76) single_step();
#ifdef B_DEBUG
    printf("3rd item on return stack is %d (should be %d).\n", *(RP + 2), *SP);
#endif
    if (*(RP + 2) != *SP) {
    	printf("Error in DoLoopT: EP = %ld\n", val_EP());
    	exit(1);
    }

    single_step(); single_step();
#ifdef B_DEBUG
    show_data_stack();  printf("Correct stack: %s\n\n", correct[4]);
#endif
    if (strcmp(correct[4], val_data_stack())) {
        printf("Error in DoLoopT: EP = %ld\n", val_EP());
        exit(1);
    }

    printf("DoLoopT ran OK\n");
    return 0;
}
