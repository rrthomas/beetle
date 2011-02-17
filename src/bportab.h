/* BPORTAB.H

    (c) Reuben Thomas 1994-2011

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


/* Define a macro to call the function pointed to by the top item(s) on Beetle's
   data stack and increment the stack pointer to drop the pointer */

#define LINK SP++; (*(void (*)(void))*(SP - 1))()


/* Define unbuffered I/O macros */

#define GETCH getchar()
#define PUTCH(c) putchar(c)
#define NEWL putchar('\n')


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
