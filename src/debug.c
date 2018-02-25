/* DEBUG.C

    (c) Reuben Thomas 1994-2018

    Functions useful for debugging Beetle.

*/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "xvasprintf.h"
#include "beetle.h"	/* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "debug.h"	/* the header we're implementing */


int instrs = 0; /* number of instructions assembled */
int ibytes; /* number of opcodes assembled in current instruction word so far */
static CELL icell;  /* accumulator for instructions being assembled */
UCELL current;	/* where the current instruction word will be stored */
UCELL here; /* where we assemble the next instruction word or literal */
UCELL S0;
UCELL R0;


void ass(BYTE instr)
{
    icell |= instr << ibytes * 8;
    instrs++;  ibytes++;
    if (ibytes == CELL_W) {
        beetle_store_cell(current, icell);  current = here;  here += CELL_W;
        icell = 0;  ibytes = 0;  instrs++;
    }
}

void lit(CELL literal)
{
    if (ibytes == 0) { here -= CELL_W;  beetle_store_cell(here, literal);  here += CELL_W * 2; current += CELL_W; }
    else { beetle_store_cell(here, literal);  here += CELL_W; }
}

void ilit(CELL literal)
{
    icell |= literal << ibytes * 8;
    beetle_store_cell(current, icell);  current = here;  here += CELL_W;
    icell = 0;  ibytes = 0;
}

void plit(void (*literal)(void))
{
    CELL_pointer address;
    unsigned i;
    address.pointer = literal;
    for (i = 0; i < POINTER_W; i++) {
        ass(O_LITERAL);
        lit(address.cells[i]);
    }
}

void start_ass(void)
{
    ibytes = 0;  icell = 0;  current = here;  here += CELL_W;
}

void end_ass(void)
{
    if (ibytes != 0) beetle_store_cell(current, icell);
    else instrs--;
}

static const char *mnemonic[] = { "NEXT00", "DUP", "DROP", "SWAP", "OVER",
    "ROT", "-ROT", "TUCK", "NIP", "PICK", "ROLL", "?DUP", ">R", "R>", "R@", "<",
    ">", "=", "<>", "0<", "0>", "0=", "0<>", "U<", "U>", "0", "1", "-1", "CELL",
    "-CELL", "+", "-", ">-<", "1+", "1-", "CELL+", "CELL-", "*", "/", "MOD",
    "/MOD", "U/MOD", "S/REM", "2/", "CELLS", "ABS", "NEGATE", "MAX", "MIN",
    "INVERT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "1LSHIFT", "1RSHIFT", "@",
    "!", "C@", "C!", "+!", "SP@", "SP!", "RP@", "RP!", "BRANCH", "BRANCHI",
    "?BRANCH", "?BRANCHI", "EXECUTE", "@EXECUTE", "CALL", "CALLI", "EXIT",
    "(DO)", "(LOOP)", "(LOOP)I", "(+LOOP)", "(+LOOP)I", "UNLOOP", "J",
    "(LITERAL)", "(LITERAL)I", "THROW", "HALT", "EP@", "LIB", "LINK",
    "RUN", "STEP", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "NEXTFF" };

_GL_ATTRIBUTE_CONST const char *disass(BYTE opcode)
{
    if (mnemonic[opcode] == NULL) return "";
    return mnemonic[opcode];
}

_GL_ATTRIBUTE_PURE BYTE toass(char *token)
{
    for (int i = 0; i < 0x5c; i++)
        if (strcmp(token, mnemonic[i]) == 0) return i;

    if (strcmp(token, mnemonic[0xff]) == 0) return 0xff;

    return 0xfe;
}

char *val_data_stack(void)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");

    for (UCELL i = S0 - CELL_W; i >= SP; i -= CELL_W) {
        CELL c;
        beetle_load_cell(i, &c);
        char *ptr = xasprintf("%s%"PRId32, picture, c);
        free(picture);
        picture = ptr;
        if (i != SP) {
            ptr = xasprintf("%s ", picture);
            free(picture);
            picture = ptr;
        }
        if (i == 0)
            break;
    }

    return picture;
}

void show_data_stack(void)
{
    printf("Data stack: %s\n", val_data_stack());
}

void show_return_stack(void)
{
    printf("Return stack: ");
    for (UCELL i = R0 - CELL_W; i >= RP; i -= CELL_W) {
        CELL c;
        beetle_load_cell(i, &c);
        printf("%"PRIX32"h ", (UCELL)c);
        if (i == 0)
            break;
    }
    putchar('\n');
}
