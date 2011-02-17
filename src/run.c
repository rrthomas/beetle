/* RUN.C

    (c) Reuben Thomas 1994-1997

    The interface call run() : integer.

*/


#include <stdio.h>
#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


CELL run(void)
{
    int ret;

    while ((ret = single_step()) == -260)
        ;

    return ret;
}
