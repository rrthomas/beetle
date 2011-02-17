/* BPORTAB/SYSV.H

    Vrsn  Date   Comment
    ----|-------|--------------------------------------------------------------
    0.00 06jul04

    Reuben Thomas


    Header for C Beetle containing the machine-dependent types and
    definitions used in bportab.h (SYSV version).

    This header should never be #included, as it is automatically included by
    bportab.h.

*/


#ifndef BEETLE_BPORTAB_SYSV
#define BEETLE_BPORTAB_SYSV


/* GCC has arithmetic right shift, and division is symmetric */


/* Define a macro to call the function pointed to by the top item(s) on Beetle's
   data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#define GETCH getch()  /* defined in noecho.c */
#define PUTCH(c) putch(c)  /* defined in noecho.c */
#define NEWL putch('\n')  /* defined in noecho.c */


#endif
