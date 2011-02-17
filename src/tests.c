/* TESTS.C

    Vrsn  Date   Comment
    ----|-------|--------------------------------------------------------------
    0.00 18jan95
    0.01 19jan95 Error messages added and division test debugged.
    0.02 17feb95 Now includes tests.h (unnecessary, but worthwhile for
                 consistency checks).
    0.03 12jun96 Changed error messages for ENDISM check.
    0.04 06jul04 BIG_ENDIAN renamed to BEETLE_BIG_ENDIAN.
    0.05 17feb11 Fix overflow check.

    Reuben Thomas


    Test that C Beetle has been compiled properly. The following tests are run:

    1. The C compiler uses twos-complement arithmetic.
    2. BYTE, CELL and UCELL are correctly set.
    3. ARSHIFT performs an arithmetic right shift on signed quantities.
    4. The C compiler performs ordinary modular arithmetic on overflow.
    5. MOD and DIV perform floored division.
    6. ENDISM is set correctly.

*/


#include <stdio.h>
#include <limits.h>
#include "beetle.h"
#include "tests.h"


int twos_complement(void)
{
    if (!(-1) != 0) {
        printf("Beetle cannot work as the C compiler does not use "
            "twos-complement arithmetic\n");
        return 1;
    }
    return 0;
}

int types(void)
{
    int ret = 0;

    if (sizeof(BYTE) != 1 || (BYTE)(0xff) < 0) {
        printf("Type BYTE is incorrectly defined in bportab.h\n");
        ret = 1;
    }

    if (sizeof(CELL) != 4 || (CELL)(0x80000000) > 0) {
        printf("Type CELL is incorrectly defined in bportab.h\n");
        ret = 1;
    }

    if (sizeof(UCELL) != 4  || (UCELL)(0x80000000) < 0) {
        printf("Type UCELL is incorrectly defined in bportab.h\n");
        ret = 1;
    }

    return ret;
}

int arshift(void)
{
    CELL x = -1;

#ifdef LRSHIFT
    if (x >> 1 == -1) {
        printf("LRSHIFT could be undefined in bportab.h\n");
#else
    if (ARSHIFT(x, 1) != -1) {
        printf("LRSHIFT should be defined in bportab.h\n");
#endif
        return 1;
    }
    return 0;
}

int overflow(void)
{
    long x = LONG_MAX;
    long y;

    y = x + 1;
    if (y == LONG_MIN) {
        y = x - (-1);
        if (y == LONG_MIN) {
            y = x * 2;
            if (y == -2) return 0;
        }
    }
    printf("Beetle cannot work as the C compiler does not use "
           "modular arithmetic overflow\n");
    return 1;
}

int division(void)
{
    CELL x = -10, y = 7, t;

    if (DIV(x, y) != -2 || MOD(x, y, t) != 4) {
#ifdef FLOORED
        printf("FLOORED should be undefined in bportab.h\n");
#else
        printf("FLOORED should be defined in bportab.h\n");
#endif
        return 1;
    }
    return 0;
}

int endism(void)
{
    CELL x = 1;
    BYTE *p = (BYTE *)&x;

    if (!((*p == 1) ^ ENDISM)) {
#ifdef BEETLE_BIG_ENDIAN
        printf("BEETLE_BIG_ENDIAN should not be defined\n");
#else
        printf("BEETLE_BIG_ENDIAN should be defined\n");
#endif
        return 1;
    }
    return 0;
}


int tests(void)
{
    return (twos_complement() | types() | arshift() | overflow() | division() |
        endism());
}
