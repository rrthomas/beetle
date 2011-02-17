/* EXCEPTS.C

    (c) Reuben Thomas 1995-1996

    Deal with address exceptions during execution cycle. Included by step.c and
    run.c.

*/


invadr:	SP--; if ((UCELL)((BYTE *)SP - M0) >= MEMORY || (unsigned int)SP & 3)
        return -258; *SP = -9; goto throw;
aliadr: SP--; if ((UCELL)((BYTE *)SP - M0) >= MEMORY || (unsigned int)SP & 3)
        return -258; *SP = -23; goto throw;
