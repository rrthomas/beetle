/* STEP.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 09nov94 Skeleton code.
    0.01 10nov94 Now use opcodes.h.
    0.02 11nov94 Added code for DUP.
    0.03 14nov94 Added code for DROP, SWAP, OVER, ROT, -ROT, TUCK and NIP,
    	    	 and explicit case in I=A assignment.
    0.04 16nov94 Added code for PICK and ROLL.
    0.05 17nov94 Added code for ?DUP, >R, R>, R@, 0, 1, -1, CELL, -CELL, +, -,
    	    	 >-<, 1+, 1-, CELL+, CELL-, *.
    0.06 19nov94 Added code for /, MOD, /MOD, 2/, CELLS, ABS, NEGATE, MAX, MIN,
    	    	 INVERT, AND, OR, XOR, LSHIFT, RSHIFT, 1LSHIFT, 1RSHIFT.
    0.07 20nov94 Debugged MIN, and removed dummy entries for double-cell
    	    	 division instructions.
    0.08 21nov94 Added code for (LITERAL) and (LITERAL)I.
    0.09 22nov94 Added code for <, >, =, <>, 0<, 0>, 0=, U<, U>, @, !, C@, C!,
    	    	 +!, SP@, SP!, RP@ and RP!.
    0.10 23nov94 Debugged !, C!, +!, SP@ and RP@, and changed SP! and RP! to
    	    	 match. Optimised memory access instructions. Added code for
    	    	 BRANCH, BRANCHI, ?BRANCH, ?BRANCHI, EXECUTE, @EXECUTE, CALL,
    	    	 CALLI and EXIT.
    0.11 24nov94 Debugged ?BRANCHI. Added code for (DO), J and (CREATE).
    0.12 25nov94 Supplied missing bracket in (CREATE). Added code for (THROW),
    	    	 HALT, LINK and LIB. Changed return type of single_step to long.
    	    	 Added the lib function, to provide the standard library
    	    	 accessed by LIB. Added exception -256 on invalid opcode.
    0.13 26nov94 Added code for (LOOP), (LOOP)I, (LEAVE), (LEAVE)I and (LEAP).
    0.14 27nov94 Changed J to match debugged specification. Added code for
    	    	 (+LOOP) and (+LOOP)I. temp's type changed to CELL, so that it
    	    	 can hold all the values which may be assigned to it.
    0.15 29nov94 Added return value in normal case to match specification, and
    	    	 make the program correct. RSHIFT changed to be ANSI C.
    0.16 30nov94 Debugged RSHIFT and 1RSHIFT.
    0.17 01dec94 Changed return type of single_step to CELL. RSHIFT and 1RSHIFT
    	    	 cast their results to CELL before assignment. Increments and
    	    	 decrements applied to stack operators separated from
    	    	 expressions using their values more than once to guarantee
    	    	 correct evaluation. +, - and * made more efficient. LSHIFT and
    	    	 RSHIFT debugged. C@ and C! changed to comply with debugged
    	    	 specification.
    0.18 11jan95 Added 0<>, U/MOD and S/REM; removed (LEAVE) and (LEAVE)I;
                 changed (LEAP) to UNLOOP. i's type changed to CELL to
                 correspond with assignments made to it.
    0.19 13jan95 Debugged LINK.
    0.20 05feb95 Changed return type of HALT instruction to CELL, which is what
    	    	 beetle.h says it should be. Altered LINK as specification
    	    	 changed.
    0.21 23mar95 Added address checking.
    0.22 24mar95 Removed contents of single_step to execute.c, and lib function
    	    	 to lib.c, which are now #included. lib.c now #included by
    	    	 execute.c.
    0.23 25mar95 Removed unnecessary reference to bintern.h.
    0.24 19apr95 Added inclusion of stdio.h.
    0.25 30mar97 Added inclusion of lib.h.

    Reuben Thomas


    The interface call single_step() : integer.

*/


#include <stdio.h>
#include "beetle.h" 	/* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


CELL single_step(void)
{
#include "execute.c"	/* one pass of execution cycle */
    return 0;	/* terminated OK */
#include "excepts.c"	/* code for address exceptions */
}
