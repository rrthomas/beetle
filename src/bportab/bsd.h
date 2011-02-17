/* BPORTAB/BSD.H

    Vrsn  Date   Comment
    ----|-------|--------------------------------------------------------------
    0.00 06jun96
    0.01 06jul04 Moved to bsd.h because of new sysv.h.

    Reuben Thomas


    Header for C Beetle containing the machine-dependent types and
    definitions used in bportab.h (BSD version).

    This header should never be #included, as it is automatically included by
    bportab.h.

*/


#ifndef BEETLE_BPORTAB_BSD
#define BEETLE_BPORTAB_BSD


/* Types required by CBeetle */

typedef unsigned char BYTE;	/* should work on most compilers */
typedef signed int CELL;	/* ditto */
typedef unsigned int UCELL;	/* ditto */


/* GCC has arithmetic right shift, and division is symmetric */


/* Define a macro to call the function pointed to by the top item(s) on Beetle's
   data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#define GETCH getchar()
#define PUTCH(c) putchar(c)
#define NEWL putchar('\n')


#endif
