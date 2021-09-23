// Test LIB instruction. Also uses previously-tested instructions.
// FIXME: test file routines.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
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
    ass(O_LITERALI); ilit(16);
    ass(O_LIB); ilit(0); /* pad word with NEXT */
    ass(O_ONE); ass(O_LITERALI); ilit(17);
    ass(O_LIB); ilit(0); /* pad word with NEXT */
    ass(O_ONE); ass(O_LITERAL); lit(buf); ass(O_LITERALI); ilit(18);
    ass(O_LIB); ilit(0); /* pad word with NEXT */

    assert(single_step() == -259);   // load first instruction word

    assert(single_step() == -259);
    assert(single_step() == -259);
    assert(single_step() == -259);
    printf("argc is %"PRId32", and should be %d\n\n", LOAD_CELL(SP), argc);
    if (POP != argc) {
       printf("Error in LIB tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(single_step() == -259);
    assert(single_step() == -259);
    assert(single_step() == -259);
    assert(single_step() == -259);
    printf("arg 1's length is %"PRId32", and should be %zu\n", LOAD_CELL(SP), strlen(argv[1]));
    if ((UCELL)POP != strlen(argv[1])) {
        printf("Error in LIB tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(single_step() == -259);
    assert(single_step() == -259);
    assert(single_step() == -259);
    assert(single_step() == -259);
    printf("arg is %s, and should be %s\n", native_address_of_range(buf, 0), argv[1]);
    if (strncmp((char *)native_address_of_range(buf, 0), argv[1], strlen(argv[1])) != 0) {
        printf("Error in extra instructions tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(exception == 0);
    printf("LIB tests ran OK\n");
    return 0;
}
