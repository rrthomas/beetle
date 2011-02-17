/* EXCEPTS.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 24mar95 Code removed from execute.c v0.00. See that file for earlier
    	    	 history. Added code to handle division by zero exception.
    0.01 25mar95 Added code to HALT if SP is unaligned. Removed division by zero
    	    	 code which properly belongs in execute.c. Corrected return code
    	    	 when SP unaligned.
    0.02 20may96 Made MEMORY an invalid address (>MEMORY -> >=MEMORY).
    0.03 26may96 Corrected return code for bad SP in aliadr to -258.

    Reuben Thomas


    Deal with address exceptions during execution cycle. Included by step.c and
    run.c.

*/


invadr:	SP--; if ((UCELL)((BYTE *)SP - M0) >= MEMORY || (unsigned int)SP & 3)
    	return -258; *SP = -9; goto throw;
aliadr: SP--; if ((UCELL)((BYTE *)SP - M0) >= MEMORY || (unsigned int)SP & 3)
    	return -258; *SP = -23; goto throw;
