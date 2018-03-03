/* SAVEOBJT.C

    (c) Reuben Thomas 1995-2018

    Test save_object().

*/


#include "btests.h"


int correct[] = { -1, -1, 0 };


static int try(const char *file, UCELL address, UCELL length)
{
    FILE *fp = fopen(file, "w");
    int ret = save_object(fp, address, length);

    printf("save_object(\"%s\", %"PRIu32", %#"PRIX32") returns %d", file,
           address, length, ret);
    fclose(fp);

    return ret;
}


int main(void)
{
    int exception = 0;
    int i;
    CELL adr[] = { 0, 0, 0 };
    UCELL len[] = { 16, 2000, 16 };

    size_t size = 256;
    init_beetle((CELL *)calloc(size, sizeof(CELL)), size);
    adr[0] = MEMORY + CELL_W;
    beetle_store_cell(0, 0x01020304);
    beetle_store_cell(4, 0x05060708);

    for (i = 0; i < 3; i++) {
        int res = try("saveobj", adr[i], len[i]);
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
        int ret = load_object(fp, 16);

        if (ret) {
            printf("Error in SaveObjT: %d returned by load_object\n", ret);
            exit(1);
        }

        fclose(fp);
        remove("saveobj");
    }
    for (i = 0; i < 4; i++) {
        CELL old, new;
        beetle_load_cell(i, &old);
        beetle_load_cell(i + 16, &new);
        printf("Word %d of memory is %"PRIX32"; should be %"PRIX32"\n", i,
               (UCELL)new, (UCELL)old);
        if (new != old) {
            printf("Error in SaveObjT: loaded file does not match data "
                   "saved\n");
            exit(1);
        }
    }

    assert(exception == 0);
    printf("SaveObjT ran OK\n");
    return 0;
}
