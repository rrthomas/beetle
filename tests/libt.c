/* LIBT.C

    (c) Reuben Thomas 1994-2018

    Also uses previously-tested instructions. The value of BL is printed, a
    number of CRs are printed, and some text is accepted by KEY and then
    EMITted.
    FIXME: test routines [4, 12] from Forth.

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
    /* Data for ARGC/ARG tests */
    int argc = 3;
    char *argv[] = {strdup("foo"), strdup("bard"), strdup("basilisk")};

    init_beetle((CELL *)malloc(4096), 1024, 16);
    assert(register_args(argc, argv));
    here = EP;
    S0 = SP;	/* save base of stack */
    R0 = RP;	/* save base of return stack */

    start_ass();
    ass(O_ZERO); ass(O_LIB); ass(O_ONE); ass(O_LIB);
    ass(O_ONE); ass(O_LIB); ass(O_ONE); ass(O_LIB);
    ass(O_LITERALI); ilit(3);
    ass(O_LIB); ass(O_DUP); ass(O_LITERALI); ilit(10);
    ass(O_EQUAL); ass(O_QBRANCHI); ilit(-3);
    ass(O_DROP); ass(O_SPFETCH); ass(O_LITERAL); lit((CELL)(S0 - M0) * CELL_W);
        ass(O_SWAPMINUS);
    ass(O_CELL); ass(O_SLASH); ass(O_ZERO); ass(O_DO);
    ass(O_LITERALI); ilit(2);
    ass(O_LIB); ass(O_LOOPI); ilit(-2);
    ass(O_ONE); ass(O_LIB);
    ass(O_LITERALI); ilit(13);
    ass(O_LIB); ilit(0); /* pad out word with NEXT (00h) */
    ass(O_ONE); ass(O_LITERALI); ilit(14);
    ass(O_LIB); ilit(0); /* pad out word with NEXT (00h) */
    end_ass();

    NEXT;   /* load first instruction word */

    single_step();  single_step();
    printf("BL should be %d, and is %d\n\n", (unsigned char)(' '), *SP);
    if (*SP++ != (unsigned char)(' ')) {
        printf("Error in LibT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    for (int i = 0; i < 3; i++) {
        printf("%d", i);
        single_step();  single_step();
        if (i == 0) single_step();
    }
    printf("\n0 1 2 should be at the start of separate lines.\n"
        "N.B. this is NOT automatically checked!\n\n");

    printf("Type some text (may not be echoed) terminated with a linefeed. It"
        "\nshould be displayed with the characters in reverse order.\n\n");
    while (val_EP() < 60)
        single_step();
    putchar('\n');

    while (val_EP() < 68)
        single_step();
    printf("argc is %"PRId32", and should be %d\n\n", *SP, argc);
    if (*SP++ != argc) {
        printf("Error in LibT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    while (val_EP() < 76)
        single_step();
    printf("arg 1's length is %"PRId32", and should be %zu\n", *SP, strlen(argv[1]));
    if ((UCELL)*SP++ != strlen(argv[1])) {
        printf("Error in LibT: EP = %"PRId32"\n", val_EP());
        exit(1);
    }

    printf("\nLibT ran OK (unless one of the unchecked tests failed - see "
        "above)\n");
    return 0;
}
