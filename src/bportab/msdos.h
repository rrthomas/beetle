/* BPORTAB/MSDOS.H

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 05jun96

    Reuben Thomas


    Header for C Beetle containing the machine-dependent types and
    definitions used in bportab.h (MSDOS version).

    This header should never be #included, as it is automatically included by
    bportab.h.

*/


#ifndef BEETLE_BPORTAB_MSDOS
#define BEETLE_BPORTAB_MSDOS


/* GCC has arithmetic right shift, and division is symmetric. */


/* Define a macro to call the function pointed to by the top item(s) on
   Beetle's data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#undef unix
#include <conio.h>
#define GETCH getch()
#define PUTCH(c) putch((int)(c))
#define NEWL putchar('\n')


#endif
