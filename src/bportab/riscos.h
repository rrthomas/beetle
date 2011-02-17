/* BPORTAB/RISCOS.H

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 05jun96
    0.01 25may97 Changed to use SharedCLib.

    Reuben Thomas


    Header for C Beetle containing the machine-dependent types and
    definitions used in bportab.h (RISC OS version).

    This header should never be #included, as it is automatically included by
    bportab.h.

*/


#ifndef BEETLE_BPORTAB_RISCOS
#define BEETLE_BPORTAB_RISCOS


/* GCC has arithmetic right shift, and division is symmetric */


/* Define a macro to call the function pointed to by the top item(s) on
   Beetle's data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#include <kernel.h>
#include <swis.h>
#define GETCH _kernel_osrdch()
#define PUTCH(c) _kernel_oswrch(c)
#define NEWL _kernel_swi(OS_NewLine, NULL, NULL)


#endif
