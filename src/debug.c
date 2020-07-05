// Functions useful for VM debugging.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "external_syms.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "verify.h"
#include "xvasprintf.h"

#include "beetle.h"
#include "beetle_aux.h"
#include "beetle_debug.h"
#include "beetle_opcodes.h"


static int ibytes; // number of opcodes assembled in current instruction word so far
static CELL icell;  // accumulator for instructions being assembled
static UCELL current;	// where the current instruction word will be stored
static UCELL here; // where we assemble the next instruction word or literal


// Return number of bytes required for a CELL-sized quantity
// After https://stackoverflow.com/questions/2589096/find-most-significant-bit-left-most-that-is-set-in-a-bit-array
verify(CELL_BIT == 32); // Code is hard-wired for 32 bits
_GL_ATTRIBUTE_CONST int byte_size(CELL v)
{
    static const int pos[32] = {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    if (v < 0)
        v = -v;

    v |= v >> 1; // first round down to one less than a power of 2
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    return pos[(UCELL)(v * 0x07C4ACDDU) >> 27] / 8 + 1;
}

void ass(BYTE instr)
{
    icell |= instr << ibytes * 8;
    store_cell(current, icell);
    ibytes++;
    if (ibytes == CELL_W) {
        current = here;  here += CELL_W;
        icell = 0;  ibytes = 0;
    }
}

void lit(CELL literal)
{
    if (ibytes == 0) { store_cell(here - CELL_W, literal);  current += CELL_W; }
    else { store_cell(here, literal); }
    here += CELL_W;
}

bool ilit(CELL literal)
{
    if (byte_size(literal) > CELL_W - ibytes)
        return false;

    icell |= literal << ibytes * 8;
    store_cell(current, icell);  current = here;  here += CELL_W;
    icell = 0;  ibytes = 0;
    return true;
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

void start_ass(UCELL addr)
{
    here = addr;  ibytes = 0;  icell = 0;  current = here;  here += CELL_W;
}

_GL_ATTRIBUTE_PURE UCELL ass_current(void)
{
    return current;
}

static const char *mnemonic[UINT8_MAX + 1] = {
    "NEXT00", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "TUCK",
    "NIP", "PICK", "ROLL", "?DUP", ">R", "R>", "R@", "<",
    ">", "=", "<>", "0<", "0>", "0=", "0<>", "U<",
    "U>", "0", "1", "-1", "CELL", "-CELL", "+", "-",
    ">-<", "1+", "1-", "CELL+", "CELL-", "*", "/", "MOD",
    "/MOD", "U/MOD", "S/REM", "2/", "CELLS", "ABS", "NEGATE", "MAX",
    "MIN", "INVERT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "1LSHIFT",
    "1RSHIFT", "@", "!", "C@", "C!", "+!", "SP@", "SP!",
    "RP@", "RP!", "EP@", "S0@", "S0!", "R0@", "R0!", "'THROW@",
    "'THROW!", "MEMORY@", "'BAD@", "-ADDRESS@", "BRANCH", "BRANCHI", "?BRANCH", "?BRANCHI",
    "EXECUTE", "@EXECUTE", "CALL", "CALLI", "EXIT", "(DO)", "(LOOP)", "(LOOP)I",
    "(+LOOP)", "(+LOOP)I", "UNLOOP", "J", "(LITERAL)", "(LITERAL)I", "THROW", "HALT",
    "LINK", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "ARGC", "ARGLEN", "ARGCOPY", "STDIN_FILENO", "STDOUT_FILENO", "STDERR_FILENO", "OPEN-FILE", "CLOSE-FILE",
    "READ-FILE", "WRITE-FILE", "FILE-POSITION", "REPOSITION-FILE", "FLUSH-FILE", "RENAME-FILE", "DELETE-FILE", "FILE-SIZE",
    "RESIZE-FILE", "FILE-STATUS", NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, "NEXTFF" };

_GL_ATTRIBUTE_CONST const char *disass(BYTE opcode)
{
    if (mnemonic[opcode] == NULL) return "";
    return mnemonic[opcode];
}

_GL_ATTRIBUTE_PURE BYTE toass(const char *token)
{
    for (size_t i = 0; i < sizeof(mnemonic) / sizeof(mnemonic[0]); i++)
        if (mnemonic[i] && strcmp(token, mnemonic[i]) == 0) return i;

    return O_UNDEFINED;
}

static char *_val_data_stack(bool with_hex)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");
    if (!STACK_UNDERFLOW(SP, S0))
        for (UCELL i = S0; i != SP;) {
            CELL c;
            char *ptr;
            i += CELL_W * STACK_DIRECTION;
            int exception = load_cell(i, &c);
            if (exception != 0) {
                ptr = xasprintf("%sinvalid address!", picture);
                free(picture);
                picture = ptr;
                break;
            }
            ptr = xasprintf("%s%"PRId32, picture, c);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%"PRIX32") ", picture, (UCELL)c);
                free(picture);
                picture = ptr;
            }
            if (i != SP) {
                ptr = xasprintf("%s ", picture);
                free(picture);
                picture = ptr;
            }
        }

    return picture;
}

char *val_data_stack(void)
{
    return _val_data_stack(false);
}

void show_data_stack(void)
{
    if (SP == S0)
        printf("Data stack empty\n");
    else if (STACK_UNDERFLOW(SP, S0))
        printf("Data stack underflow\n");
    else
        printf("Data stack: %s\n", _val_data_stack(true));
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
            int exception = load_cell(i, &c);
            if (exception != 0) {
                printf("invalid address!\n");
                break;
            }
            printf("$%"PRIX32" ", (UCELL)c);
        }
        putchar('\n');
    }
}
