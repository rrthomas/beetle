/* BPORTAB/TOS.H

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 29mar97 Adapted from msdos.h v0.00
    0.01 17feb11 Rename BIG_ENDIAN to BEETLE_BIG_ENDIAN to match bportab.h.

    Reuben Thomas


    Header for C Beetle containing the machine-dependent types and
    definitions used in bportab.h (Atari TOS version).

    This header should never be #included, as it is automatically included by
    bportab.h.

*/


#ifndef BEETLE_BPORTAB_TOS
#define BEETLE_BPORTAB_TOS


/* Types required by CBeetle */

typedef unsigned char BYTE;     /* should work on most compilers */
typedef signed long CELL;       /* ditto */
typedef unsigned long UCELL;    /* ditto */


/* 680x0 processors are big-endian */

#define BEETLE_BIG_ENDIAN


/* GCC has arithmetic right shift, and division is symmetric. */


/* Define a macro to call the function pointed to by the top item(s) on
   Beetle's data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#undef unix
#include <conio.h>
#define GETCH getch()
#define PUTCH(c) putch((int)(c))
#define NEWL putch(13); putch(10)


#endif
