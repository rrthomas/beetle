/* BPORTAB.H

    Vrsn  Date   Comment
    ----|-------|--------------------------------------------------------------
    0.00 07nov94
    0.01 09nov94 ENDISM given an explicit type, as per specification, and
                 ARSHIFT macro added.
    0.02 18nov94 SYMMETRIC added, and definitions of floored division and
                 remainder to work on symmetric division compilers (most of
                 them!).
    0.03 19nov94 Corrected MOD, and added ABS and SGN to do so.
    0.04 25nov94 Added LINK macro to implement the LINK instruction.
    0.05 01dec94 FLIP macro added to comply with debugged specification. ENDISM
                 made dependent on whether BIG_ENDIAN is defined.
    0.06 12jan95 Division macros changed so that both symmetric and floored
                 division are provided in accordance with modified Beetle
                 specification.
    0.07 18jan95 SYMMETRIC changed to FLOORED, and made valueless. Added
                 LRSHIFT, and #defines for BIG_ENDIAN, FLOORED and LRSHIFT.
    0.08 05feb95 LINK changed to make it update SP so that the pointer can
                 occupy multiple stack items as specified.
    0.09 02apr95 Added GETCH.
    0.10 03apr95 Added PUTCH and NEWL.
    0.11 18apr95 Debugged LINK, DIV and SDIV.
    0.12 06jun96 Moved machine-dependent parts to machine-specific header files
                 in bportab/. These are now included according to the machine
                 type symbol defined.
    0.13 16jun96 Reduced number of machine types.
    0.14 06jul96 Added missing include of bportab/unix.h.
    0.15 30mar97 Removed nested comments.
    0.16 06jul04 Replaced include of bportab/unix.h with bportab/bsd.h and
                 bportab/sysv.h. Split unix into bsd and sysv. Give an error if
                 no platform macro defined. Rename BIG_ENDIAN to
                 BEETLE_BIG_ENDIAN to fix clash with system headers.

    Reuben Thomas


    Header for C Beetle defining the machine-dependent types and
    definitions used in beetle.h.

    To allow C Beetle to compile on a system not mentioned here, a new machine-
    specific header file must be written, modelled on those already existing,
    and this file must be modified to recognise the new machine name. Commented-
    out versions of the macros that must be defined are included below, as
    suggestions. The file noecho.c allows the keypress primitives to be
    implemented easily on many Unix systems.

    This header should never be #included, as it is automatically included by
    beetle.h.

    Beetle will only work if the number representation is twos complement.

*/


#ifndef BEETLE_BPORTAB
#define BEETLE_BPORTAB


#include <stdint.h>


/* Include the machine-specific definitions. These are included based on the
   system name symbol declared; those currently recognised are the symbols
   defined by GCC on the corresponding machines.

   The current machine types recognised, and the corresponding symbol(s) that
   should be defined for each type are:

   Machine type		Symbol(s)
   ------------		---------

   RISC OS		riscos
   MSDOS		MSDOS
   Atari TOS            atarist
   BSD                  bsd
   SysV (e.g. Linux)    sysv
*/

#ifdef riscos
    #include "bportab/riscos.h"
#elif defined(MSDOS)
    #include "bportab/msdos.h"
#elif defined(atarist)
    #include "bportab/tos.h"
#elif defined(bsd)
    #include "bportab/bsd.h"
#elif defined(sysv)
    #include "bportab/sysv.h"
#else
    #error "No platform defined!"
#endif


/* Types required by CBeetle: BYTE should be an unsigned eight-bit quantity,
   CELL a signed four-byte quantity, and UCELL an unsigned CELL. */

typedef uint8_t BYTE;
typedef int32_t CELL;
typedef uint32_t UCELL;


/* Define the value of ENDISM. This is fixed at compile-time, which seems
   reasonable, as machines rarely change endianness while switched on!

   0 = Little-endian, 1 = Big-endian

   Assume big-endian if BEETLE_BIG_ENDIAN defined; little-endian
   otherwise. Also define FLIP macro for byte addressing. */

/* #define BEETLE_BIG_ENDIAN */

#ifdef BEETLE_BIG_ENDIAN
    #define ENDISM ((BYTE)1)
    #define FLIP(addr) ((addr) ^ 3)
#else
    #define ENDISM ((BYTE)0)
    #define FLIP(addr) (addr)
#endif


/* Define a macro for arithmetic right shifting (the ANSI standard does not
   guarantee that this is possible with >>) */

/* #define LRSHIFT */

#ifdef LRSHIFT
    #define ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (p)))
#else
    #define ARSHIFT(n, p) ((n) >>= (p))
#endif


/* Define division macros. FLOORED should be defined if the C compiler uses
   floored division (and I've not found one that does yet). */

/* #define FLOORED */

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SGN(x) ((x) > 0 ? 1 : -1 )  /* not a proper sign function! */

#ifdef FLOORED
    #define DIV(a, b) (a / b)
    #define MOD(a, b, t) (a % b)
    #define SDIV(a, b) ((a) / (b) + ((((a) ^ (b)) < 0) && ((a) % (b)) != 0))
    #define SMOD(a, b, t) (t = (a) % (b), (((a) ^ (b)) >= 0 || t == 0)) ? t : \
        SGN(a) * (ABS(b)-ABS(t))
#else
    #define DIV(a, b) ((a) / (b) - ((((a) ^ (b)) < 0) && ((a) % (b)) != 0))
    #define MOD(a, b, t) (t = (a) % (b), (((a) ^ (b)) >= 0 || t == 0)) ? t : \
        SGN(b) * (ABS(b)-ABS(t))
    #define SDIV(a, b) (a / b)
    #define SMOD(a, b, t) (a % b)
#endif


/* Define a macro to call the function pointed to by the top item(s) on Beetle's
   data stack and increment the stack pointer to drop the pointer */

/* #define LINK SP++; (*(void (*)(void))*(SP - 1))() */


/* Define a macro to return a keypress immediately and without echo */

/* #define GETCH ??? */
/* #define PUTCH(c) ??? */
/* #define NEWL ??? */


#endif
