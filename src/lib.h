/* LIB.H

    (c) Reuben Thomas 1997-2018

    Prototypes for lib functions.

*/


#ifndef BEETLE_LIB
#define BEETLE_LIB

#include "config.h"

#include <stdbool.h>

#include "beetle.h"

void lib(UCELL);
#define LIB_ROUTINES 14

extern int main_argc;
extern UCELL *main_argv;
extern UCELL *main_argv_len;
bool register_args(int argc, char *argv[]);

#endif
