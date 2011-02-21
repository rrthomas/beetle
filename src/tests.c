/* TESTS.C

    (c) Reuben Thomas 1995-2011

    Test that C Beetle has been compiled properly. The following tests are run:

    1. The C compiler uses twos-complement arithmetic.
    2. The C compiler performs modular reduction on overflow of signed types.

*/


#include "config.h"

#include <stdio.h>
#include <limits.h>
#include "beetle.h"
#include "tests.h"


static int twos_complement(void)
{
    if (~(-1) != 0) {
        printf("Beetle cannot work as the C compiler does not use "
            "twos-complement arithmetic.\n");
        return 1;
    }
    return 0;
}

static int overflow(void)
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
           "modular reduction on overflow of signed types.\n");
    return 1;
}

int tests(void)
{
    return (twos_complement() | overflow());
}
