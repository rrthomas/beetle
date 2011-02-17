/* LINKT.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 18apr95

    Reuben Thomas


    Test the LINK instruction. Assumes 4-byte addresses.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"  	/* debugging functions */


void test(void)
{
    *--SP = 37;
}

int main(void)
{
    CELL res;

    init_beetle((BYTE *)malloc(16384), 4096, 16);
    S0 = SP;	/* save base of stack */

    here = EP;	/* start assembling at 16 */
    start_ass();
    ass(O_LITERAL); lit((CELL)(test)); ass(O_LINK); ass(O_ZERO); ass(O_HALT);
    end_ass();

    NEXT;   /* load first instruction word */
    res = run();
    if (res != 0) {
        printf("Error in LinkT: test aborted with return code %ld\n", res);
        exit(1);
    }

#ifdef B_DEBUG
        printf("Top of stack is %d; should be %d\n", *SP, 37);
        show_data_stack();
        printf("%p\n", test);
#endif
    	if (*SP != 37) {
    	    printf("Error in LinkT: incorrect value on top of stack\n");
    	    exit(1);
    	}

    printf("LinkT ran OK\n");
    return 0;
}
