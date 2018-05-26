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

#include "btests.h"


int main(void)
{
    int exception = 0;
    CELL temp = 0;

    // Data for ARGC/ARG tests
    int argc = 3;
    char *argv[] = {strdup("foo"), strdup("bard"), strdup("basilisk")};

    init_beetle((CELL *)malloc(4096), 1024);
    assert(register_args(argc, argv));

    start_ass(EP);
    ass(OX_ARGC); ass(O_ONE); ass(OX_ARG);

    NEXT;   // load first instruction word

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

    printf("Extra instructions tests ran OK\n");
    return 0;
}
