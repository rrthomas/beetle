/* SAVEOBJT.C

    (c) Reuben Thomas 1995-2011

    Test save_object().

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "beetle.h"     /* main header */


int correct[] = { -1, -1, 0 };


static int try(char *file, CELL *address, UCELL length)
{
    FILE *fp = fopen(file, "w");
    int ret = save_object(fp, address, length);

    printf("save_object(\"%s\", M0 + %td, %#"PRIX32") returns %d", file,
        (BYTE *)address - M0, length, ret);
    fclose(fp);

    return ret;
}


int main(void)
{
    int i, res;
    CELL adr[] = { 0, 0, 0 };
    UCELL len[] = { 16, 2000, 16 };

    init_beetle((BYTE *)calloc(1024, 1), 256, 4);
    adr[0] = MEMORY + CELL_W;
    ((CELL *)M0)[0] = 0x01020304;
    ((CELL *)M0)[1] = 0x05060708;

    for (i = 0; i < 3; i++) {
        res = try("saveobj", (CELL *)(M0 + adr[i]), len[i]);
        if (i != 2)
          remove("saveobj");
        printf(" should be %d\n", correct[i]);
        if (res != correct[i]) {
            printf("Error in SaveObjT test %d\n", i + 1);
            exit(1);
        }
    }

    {
        FILE *fp = fopen("saveobj", "r");
        int ret = load_object(fp, (CELL *)(M0 + 16));

        if (ret) {
            printf("Error in SaveObjT: %d returned by load_object\n", ret);
            exit(1);
        }

        fclose(fp);
        remove("saveobj");
    }
    for (i = 0; i < 4; i++) {
        printf("Word %d of memory is %"PRIX32"; should be %"PRIX32"\n", i,
            ((CELL *)M0)[i + 4], ((CELL *)M0)[i]);
        if (((CELL *)M0)[i + 4] != ((CELL *)M0)[i]) {
            printf("Error in SaveObjT: loaded file does not match data "
                "saved\n");
            exit(1);
        }
    }

    printf("SaveObjT ran OK\n");
    return 0;
}
