/* STEP.C

    (c) Reuben Thomas 1994-1997

    The interface call single_step() : integer.

*/


#include <stdio.h>
#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


CELL single_step(void)
{
#include "execute.c"	/* one pass of execution cycle */
    return 0;	/* terminated OK */
#include "excepts.c"	/* code for address exceptions */
}
