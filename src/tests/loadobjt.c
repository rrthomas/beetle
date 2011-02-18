/* LOADOBJT.C

    (c) Reuben Thomas 1995-2011

    Test load_object().

*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "beetle.h"	/* main header */


static int correct[] = { -2, -2, -1, -3, 0, 0 };


static int try(char *file, CELL *address)
{
    FILE *fp = fopen(file, "r");
    int ret = load_object(fp, address);

    printf("load_object(\"%s\", 0) returns %d", file, ret);
    fclose(fp);

    return ret;
}

static char *obj_name(char *prefix, char *file)
{
    char *s = malloc(strlen(prefix) + strlen(file) + 1);
    assert(s);
    strcpy(s, prefix);
    strcat(s, "/"); /* FIXME: Use gnulib's concat-filename */
    strcat(s, file);
    return s;
}

int main(int argc, char *argv[])
{
    char *files[] = { "badobj1", "badobj2", "badobj3", "badobj4", "testobj1",
        "testobj2" };
    char *prefix = argv[1];
    int i, res;

    if (argc != 2) {
        printf("Usage: loadobjt DIRECTORY\n");
        exit(1);
    }

    init_beetle((BYTE *)malloc(1024), 256, 16);

    for (i = 0; i < 4; i++) {
        char *s = obj_name(prefix, files[i]);
        res = try(s, (CELL *)M0);
        free(s);
        printf(" should be %d\n", correct[i]);
        if (res != correct[i]) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
    }

    for (; i < 6; i++) {
        char *s = obj_name(prefix, files[i]);
        res = try(s, (CELL *)M0);
        printf(" should be %d\n", correct[i]);
        printf("Word 0 of memory is %"PRIX32"; should be 1020304\n", *(CELL*)M0);
        if (*(CELL*)M0 != 0x1020304) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
        if (res != correct[i]) {
            printf("Error in LoadObjT: file %s\n", files[i]);
            exit(1);
        }
    }

    printf("LoadObjT ran OK\n");
    return 0;
}
