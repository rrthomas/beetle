// The interface call run() : integer.
//
// (c) Reuben Thomas 1994-2016

#include "config.h"

#include "beetle.h"


CELL run(void)
{
    CELL ret;

    while ((ret = single_step()) == -260)
        ;

    return ret;
}
