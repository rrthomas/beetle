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
    while (1)
#include "execute.c"
#include "excepts.c"
}
