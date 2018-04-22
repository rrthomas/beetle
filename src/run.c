// The interface call run() : integer.
//
// (c) Reuben Thomas 1994-2016
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "config.h"

#include "beetle.h"


CELL run(void)
{
    CELL ret;

    while ((ret = single_step()) == -260)
        ;

    return ret;
}
