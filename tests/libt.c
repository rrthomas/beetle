/* LIBT.C

    (c) Reuben Thomas 1994-2018

    Also uses previously-tested instructions.
    FIXME: test file routines.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "beetle.h"	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"	/* debugging functions */


int main(void)
{
    int exception = 0;
    CELL temp = 0;

    /* Data for ARGC/ARG tests */
    int argc = 3;
    char *argv[] = {strdup("foo"), strdup("bard"), strdup("basilisk")};

    init_beetle((CELL *)malloc(4096), 1024, 16);
    assert(register_args(argc, argv));
    here = EP;

    start_ass();
    ass(O_LITERALI); ilit(0);
    ass(O_LIB); ilit(0); /* pad out word with NEXT (00h) */
    ass(O_ONE); ass(O_LITERALI); ilit(1);
    ass(O_LIB); ilit(0); /* pad out word with NEXT (00h) */
    end_ass();

    NEXT;   /* load first instruction word */

    while (EP < 28)
        single_step();
    printf("argc is %"PRId32", and should be %d\n\n", LOAD_CELL(SP), argc);
    if (POP != argc) {
       printf("Error in LibT: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    while (EP < 36)
        single_step();
    printf("arg 1's length is %"PRId32", and should be %zu\n", LOAD_CELL(SP), strlen(argv[1]));
    if ((UCELL)POP != strlen(argv[1])) {
        printf("Error in LibT: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("LibT ran OK\n");
    return 0;
}
