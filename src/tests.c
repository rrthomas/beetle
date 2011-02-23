/* TESTS.C

    (c) Reuben Thomas 1995-2011

    Test that CELL is a two's complement type.

    This test is not run in configure.ac because that would preclude
    cross-compiling Beetle.

*/


#include "config.h"

#include "intprops.h"
#include "beetle.h"
#include "tests.h"

int tests(void)
{
    if (!TYPE_TWOS_COMPLEMENT(CELL)) {
        printf("Beetle cannot work as CELL is not a two's complement type.\n");
        return 1;
    }
    return 0;
}
