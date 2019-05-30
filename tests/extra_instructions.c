// Test extra instructions. Also uses previously-tested instructions.
// FIXME: test file routines.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "tests.h"


int main(void)
{
    int exception = 0;
    CELL temp = 0;

    // Data for ARGC/ARGLEN/ARGCOPY tests
    int argc = 3;
    UCELL buf = 16;
    const char *argv[] = {"foo", "bard", "basilisk"};

    init((CELL *)malloc(4096), 1024);
    assert(register_args(argc, argv) == 0);

    start_ass(EP);
    ass(OX_ARGC); ass(O_ONE); ass(OX_ARGLEN);
    ass(O_ONE); ass(O_LITERAL); lit(buf); ass(OX_ARGCOPY);

    assert(single_step() == -259);   // load first instruction word

    single_step();
    printf("argc is %"PRId32", and should be %d\n\n", LOAD_CELL(SP), argc);
    if (POP != argc) {
       printf("Error in extra instructions tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    single_step();
    single_step();
    printf("arg 1's length is %"PRId32", and should be %zu\n", LOAD_CELL(SP), strlen(argv[1]));
    if ((UCELL)POP != strlen(argv[1])) {
        printf("Error in extra instructions tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    single_step();
    single_step();
    single_step();
    single_step();
    printf("arg is %s, and should be %s\n", native_address_of_range(buf, 0), argv[1]);
    if (strcmp((char *)native_address_of_range(buf, 0), argv[1]) != 0) {
        printf("Error in extra instructions tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("Extra instructions tests ran OK\n");
    return 0;
}
