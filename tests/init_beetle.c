// Test that the Beetle headers beetle.h compiles properly, and that the
// storage allocation and register initialisation in storage.c works.
//
// (c) Reuben Thomas 1994-2018

#include "btests.h"


int main(void)
{
    int i = init_beetle((CELL *)NULL, 4);
    printf("init_beetle((CELL *)NULL, 4) should return -1; returns: %d\n", i);
    if (i != -1) {
        printf("Error in init_beetle() tests: init_beetle with invalid parameters "
            "succeeded\n");
        exit(1);
    }

    size_t size = 1024;
    CELL *ptr = (CELL *)malloc(size);
    assert(ptr);
    i = init_beetle(ptr, size / CELL_W);
    if (i != 0) {
        printf("Error in init_beetle() tests: init_beetle with valid parameters failed\n");
        exit(1);
    }

    printf("init_beetle() tests ran OK\n");
    return 0;
}
