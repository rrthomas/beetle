/* LINKT.C

    (c) Reuben Thomas 1995-2018

    Test the LINK instruction.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"      /* debugging functions */


static void test(void)
{
    *--SP = 37;
}

int main(void)
{
    CELL res;

    init_beetle((CELL *)malloc(16384), 4096, 16);
    S0 = SP;	/* save base of stack */

    here = EP;	/* start assembling at 16 */
    start_ass();
    plit(test); ass(O_LINK); ass(O_ZERO); ass(O_HALT);
    end_ass();

    NEXT;   /* load first instruction word */
    res = run();
    if (res != 0) {
        printf("Error in LinkT: test aborted with return code %"PRId32"\n", res);
        exit(1);
    }

    printf("Top of stack is %d; should be %d\n", *SP, 37);
    show_data_stack();
    if (*SP != 37) {
        printf("Error in LinkT: incorrect value on top of stack\n");
        exit(1);
    }

    printf("LinkT ran OK\n");
    return 0;
}
