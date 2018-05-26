// Functions useful for debugging Beetle.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "xvasprintf.h"

#include "beetle.h"
#include "beetle_aux.h"
#include "opcodes.h"
#include "debug.h"


int instrs = 0; // number of instructions assembled
int ibytes; // number of opcodes assembled in current instruction word so far
static CELL icell;  // accumulator for instructions being assembled
UCELL current;	// where the current instruction word will be stored
UCELL here; // where we assemble the next instruction word or literal


void ass(BYTE instr)
{
    icell |= instr << ibytes * 8;
    beetle_store_cell(current, icell);
    instrs++;  ibytes++;
    if (ibytes == CELL_W) {
        current = here;  here += CELL_W;
        icell = 0;  ibytes = 0;
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

static const char *mnemonic[UINT8_MAX + 1] = { "NEXT00", "DUP", "DROP", "SWAP", "OVER",
    "ROT", "-ROT", "TUCK", "NIP", "PICK", "ROLL", "?DUP", ">R", "R>", "R@", "<",
    ">", "=", "<>", "0<", "0>", "0=", "0<>", "U<", "U>", "0", "1", "-1", "CELL",
    "-CELL", "+", "-", ">-<", "1+", "1-", "CELL+", "CELL-", "*", "/", "MOD",
    "/MOD", "U/MOD", "S/REM", "2/", "CELLS", "ABS", "NEGATE", "MAX", "MIN",
    "INVERT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "1LSHIFT", "1RSHIFT", "@",
    "!", "C@", "C!", "+!", "SP@", "SP!", "RP@", "RP!", "EP@", "S0@", "#S", "R0@", "#R",
    "'THROW@", "'THROW!", "MEMORY@", "'BAD@", "-ADDRESS@", "BRANCH", "BRANCHI",
    "?BRANCH", "?BRANCHI", "EXECUTE", "@EXECUTE", "CALL", "CALLI", "EXIT",
    "(DO)", "(LOOP)", "(LOOP)I", "(+LOOP)", "(+LOOP)I", "UNLOOP", "J",
    "(LITERAL)", "(LITERAL)I", "THROW", "HALT", "LIB", "LINK",
    NULL, NULL, NULL,
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
    for (int i = 0; i < 0x62; i++)
        if (strcmp(token, mnemonic[i]) == 0) return i;

    if (strcmp(token, mnemonic[0xff]) == 0) return 0xff;

    return O_UNDEFINED;
}

char *val_data_stack(void)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");

    for (UCELL i = S0; i != SP;) {
        CELL c;
        char *ptr;
        i += CELL_W * STACK_DIRECTION;
        int exception = beetle_load_cell(i, &c);
        if (exception != 0) {
            ptr = xasprintf("%sinvalid address!", picture);
            free(picture);
            picture = ptr;
            break;
        }
        ptr = xasprintf("%s%"PRId32, picture, c);
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
    if (SP == S0)
        printf("Data stack empty\n");
    else if (STACK_UNDERFLOW(SP, S0))
        printf("Data stack underflow\n");
    else
        printf("Data stack: %s\n", val_data_stack());
}

void show_return_stack(void)
{
    if (RP == R0)
        printf("Return stack empty\n");
    else if (STACK_UNDERFLOW(RP, R0))
        printf("Return stack underflow\n");
    else {
        printf("Return stack: ");
        for (UCELL i = R0; i != RP;) {
            CELL c;
            i += CELL_W * STACK_DIRECTION;
            int exception = beetle_load_cell(i, &c);
            if (exception != 0) {
                printf("invalid address!\n");
                break;
            }
            printf("%"PRIX32"h ", (UCELL)c);
            if (i == 0)
                break;
        }
        putchar('\n');
    }
}
