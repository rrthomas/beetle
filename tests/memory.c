// Test the memory operators. Also uses previously tested instructions.
// See exceptions.c for address exception handling tests.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "btests.h"


const char *correct[] = {
    "", "16384", "16380", "16380 513", "16380 513 16380", "16380",
    "16380 16380", "16380 513", "16380",
    "16380 16380", "16380 1", "16381", "2", "2 16383", "", "16380", "33554945",
    "", "-33554432", "", "-16777216", "", "0", "", "0", "", "16384", "67305985",
    "", "16389", "2", "", "1", "1 16385", "", "16385", "1", "", "16392", "16392 16392", "-20",
};

const unsigned area[] = {0x4000, 0x4004, 0x4005, 0x4008};


int main(void)
{
    int exception = 0;

    // Data for extra memory area tests
    char *onetwothreefour = strdup("\x01\x02\x03\x04"); // Hold on to this to prevent a memory leak
    char *item[] = {onetwothreefour, strdup("\x01"), strdup("\x02\x03"), strdup("basilisk")};
    unsigned nitems = sizeof(item) / sizeof(item[0]);

    size_t size = 4096;
    init_beetle((CELL *)calloc(size, CELL_W), size);
    for (unsigned i = 0; i < nitems; i++) {
        UCELL addr = mem_allot(item[i], strlen(item[i]), i < 3);
        printf("Extra memory area %u allocated at address %"PRIX32" (should be %"PRIX32")\n",
               i, addr, area[i]);
        if (addr != area[i]) {
            printf("Error in memory tests: incorrect address for memory allocation\n");
            exit(1);
        }
        if (i == 2)
            mem_align();
    }

    start_ass(EP);
    ass(O_MEMORYFETCH); ass(O_MINUSCELL); ass(O_LITERAL); lit(513);
    ass(O_OVER); ass(O_STORE); ass(O_DUP); ass(O_FETCH);
    ass(O_DROP); ass(O_DUP); ass(O_CFETCH); ass(O_PLUS);
    ass(O_CFETCH); ass(O_LITERAL); lit(16383); ass(O_CSTORE); ass(O_LITERAL);
        lit(16380);
    ass(O_FETCH); ass(O_DROP); ass(O_SPFETCH); ass(O_SPSTORE);
    ass(O_RPFETCH); ass(O_DROP); ass(O_ZERO); ass(O_RPSTORE);
    ass(O_RPFETCH); ass(O_DROP);
    ass(O_LITERAL); lit(size * CELL_W); ass(O_FETCH); ass(O_DROP);
    ass(O_LITERAL); lit(size * CELL_W + 5); ass(O_CFETCH); ass(O_DROP);
    ass(O_ONE); ass(O_LITERAL); lit(size * CELL_W + 1); ass(O_CSTORE);
    ass(O_LITERAL); lit(size * CELL_W + 1); ass(O_CFETCH); ass(O_DROP);
    ass(O_LITERAL); lit(size * CELL_W + 8); ass(O_DUP); ass(O_CSTORE);

    NEXT;   // load first instruction word

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i - i / 5]);
        if (strcmp(correct[i - i / 5], val_data_stack())) {
            printf("Error in memory tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("Memory tests ran OK\n");
    return 0;
}
