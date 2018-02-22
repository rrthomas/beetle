/* DEBUG.C

    (c) Reuben Thomas 1994-2016

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
CELL *current;	/* where the current instruction word will be stored */
CELL *here; /* where we assemble the next instruction word or literal */
CELL *S0;   /* pointer to base of data stack */
CELL *R0;   /* pointer to base of return stack */


void ass(BYTE instr)
{
    icell |= instr << ibytes * 8;
    instrs++;  ibytes++;
    if (ibytes == 4) {
        *current = icell;  current = here++;
        icell = 0;  ibytes = 0;  instrs++;
    }
}

void lit(CELL literal)
{
    if (ibytes == 0) { *--here = literal; here += 2; current++; }
    else *here++ = literal;
}

void ilit(CELL literal)
{
    icell |= literal << ibytes * 8;
    *current = icell;  current = here++;
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
    ibytes = 0;  icell = 0;  current = here++;
}

void end_ass(void)
{
    if (ibytes != 0) *current = icell;
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

_GL_ATTRIBUTE_PURE CELL val_EP(void)
{
    return (EP - M0) * CELL_W;
}

char *val_data_stack(void)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");

    for (CELL *i = S0 - 1; i >= M0 + SP / CELL_W; i--) {
        char *ptr = xasprintf("%s%"PRId32, picture, *i);
        free(picture);
        picture = ptr;
        if (i != M0 + SP / CELL_W) {
            ptr = xasprintf("%s ", picture);
            free(picture);
            picture = ptr;
        }
    }

    return picture;
}

void show_data_stack(void)
{
    CELL *i;

    printf("Data stack: ");
    for (i = S0 - 1; i >= M0 + SP / CELL_W; i--)
        printf("%"PRId32" ", *i);
    putchar('\n');
}

void show_return_stack(void)
{
    CELL *i;

    printf("Return stack: ");
    for (i = R0 - 1; i >= RP; i--)
        printf("%"PRIX32"h ", (UCELL)*i);
    putchar('\n');
}
