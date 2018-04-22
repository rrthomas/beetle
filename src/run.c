/* RUN.C

    (c) Reuben Thomas 1994-2016

    The interface call run() : integer.

*/


#include "config.h"

#include "beetle.h"     /* main header */


CELL run(void)
{
    CELL ret;

    while ((ret = single_step()) == -260)
        ;

    return ret;
}
