/* LOADOBJT.C

    (c) Reuben Thomas 1995-2004

    Test load_object().

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h"	/* main header */


int correct[] = { -2, -2, -1, -3, 0, 0 };


int try(char *file, CELL *address)
{
    FILE *fp = fopen(file, "r");
    int ret = load_object(fp, address);

#ifdef B_DEBUG
    printf("load_object(\"%s\", 0) returns %d", file, ret);
#endif
    fclose(fp);

    return ret;
}


int main(void)
{
    char *files[] = { "badobj1", "badobj2", "badobj3", "badobj4", "testobj1",
        "testobj2" };
    int i, res;

    init_beetle((BYTE *)malloc(1024), 256, 16);

    for (i = 0; i < 4; i++) {
        res = try(files[i], (CELL *)M0);
#ifdef B_DEBUG
        printf(" should be %d\n", correct[i]);
#endif
        if (res != correct[i]) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
    }

    for (; i < 6; i++) {
        res = try(files[i], (CELL *)M0);
#ifdef B_DEBUG
        printf(" should be %d\n", correct[i]);
        printf("Word 0 of memory is %lx; should be 1020304\n", *(CELL*)M0);
        if (*(CELL*)M0 != 0x1020304) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
#endif
        if (res != correct[i]) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
    }

    printf("LoadObjT ran OK\n");
    return 0;
}
