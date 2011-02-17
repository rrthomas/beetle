/* LIBT.C

    (c) Reuben Thomas 1994-2011

    Also uses instructions with lower opcodes. The value of BL is printed, a
    number of CRs are printed, and some text is accepted by KEY and then
    EMITted. This test program is interdependent with doloopt.c.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h"	/* main header */
#include "btests.h"	/* Beetle tests header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"	/* debugging functions */


int main(void)
{
    int i;

    init_beetle((BYTE *)malloc(1024), 256, 16);
    here = EP;
    S0 = SP;	/* save base of stack */
    R0 = RP;	/* save base of return stack */

    start_ass();
    ass(O_ZERO); ass(O_LIB); ass(O_ONE); ass(O_LIB);
    ass(O_ONE); ass(O_LIB); ass(O_ONE); ass(O_LIB);
    ass(O_LITERALI); ilit(3);
    ass(O_LIB); ass(O_DUP); ass(O_LITERALI); ilit(10);
    ass(O_EQUAL); ass(O_QBRANCHI); ilit(-3);
    ass(O_DROP); ass(O_SPFETCH); ass(O_LITERAL); lit((CELL)((BYTE *)S0 - M0));
        ass(O_SWAPMINUS);
    ass(O_CELL); ass(O_SLASH); ass(O_ZERO); ass(O_DO);
    ass(O_LITERALI); ilit(2);
    ass(O_LIB); ass(O_LOOPI); ilit(-2);
    ass(O_ONE); ass(O_LIB);
    end_ass();

    NEXT;   /* load first instruction word */

    single_step();  single_step();
    printf("BL should be %d, and is %d\n\n", (unsigned char)(' '), *SP);
    if (*SP++ != (unsigned char)(' ')) {
        printf("Error in LibT: EP = %ld\n", val_EP());
        exit(1);
    }

    for (i = 0; i < 3; i++) {
        printf("%d", i);
        single_step();  single_step();
        if (i == 0) single_step();
    }
    printf("\n0 1 2 should be at the start of separate lines.\n"
        "N.B. this is NOT automatically checked!\n\n");

    printf("Type some text (may not be echoed) terminated with a linefeed. It"
        "\nshould be displayed with the characters in reverse order.\n\n");
    while (val_EP() < 64)
      single_step();

    printf("\nLibT ran OK (unless one of the unchecked tests failed - see "
        "above)\n");
    return 0;
}
