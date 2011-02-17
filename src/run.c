/* RUN.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 09nov94
    0.01 10nov94 Now use opcodes.h.
    0.02 25nov94 Changed return type of run to long.
    0.03 01dec94 Changed return type of run to CELL.
    0.04 24mar95 Added inclusion of code to make run functional (first working
    	    	 version). lib.c now #included by execute.c.
    0.05 25mar95 Removed unnecessary reference to bintern.h.
    0.06 30mar97 Added inclusion of lib.h.

    Reuben Thomas


    The interface call run() : integer.

*/


#include <stdio.h>
#include "beetle.h" 	/* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


CELL run(void)
{
    while (1)
#include "execute.c"
#include "excepts.c"
}
