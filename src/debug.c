/* DEBUG.C

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 17nov94
    0.01 18nov94 Made disassembly give proper bForth instruction names rather
		 than the C Beetle versions. Changed showEP to val_EP, making
		 it just return the value rather than printing it. Added
		 endass.
    0.02 20nov94 Removed double-cell division instructions from disass.
    0.03 21nov94 Added lit, and #included debug.h.
    0.04 22nov94 Added ibytes and current to allow proper assembly of
                 literals, and ilit for assembly of immediate literals; added
                 start_ass, and changed name of endass to end_ass.
    0.05 23nov94 Changed lit so that it works when the (LITERAL) instruction
                 is the last in its instruction word. Initialisation of
                 instrs moved to its declaration from start_ass, so that the
		 instruction count remains valid when pieces of code are
		 assembled in different locations with multiple calls to
		 start_ass and end_ass. Debugged setting of instrs by ilit.
		 Added "Undefined instruction" message to disass. icell now
		 set to 0 in start_ass.
    0.06 28nov94 Replaced reference to b_mem with one to M0. Made val_EP more
		 efficient. Added R0 and show_return_stack.
    0.07 29nov94 Added val_data_stack, and changed show_data_stack to use it.
    0.08 12jan95 Changed opcodes recognised by disass to match new version of
		 opcodes.h (0.02).
    0.09 17feb95 Removed the now unnecessary inclusion of bintern.h.
    0.10 18feb95 Added toass, and modified disass to use the string array
		 mnemonic, which is also used by toass.
    0.11 19feb95 Debugged toass.
    0.12 28feb95 Debugged wrong printf formats.
    0.13 02mar95 Changed output format of show_data_stack and
                 show_return_stack.
    0.14 05apr95 Increased size of string to in val_data_stack to cope with
		 large stacks; changed show_data_stack not to use
		 val_data_stack in any case.
    0.15 16apr95 Changed (THROW) to THROW, which it should be.

    Reuben Thomas


    Functions useful for debugging Beetle.

*/


#include <stdio.h>
#include <string.h>
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
    "(LITERAL)", "(LITERAL)I", "THROW", "HALT", "(CREATE)", "LIB", "OS",
    "LINK", "RUN", "STEP", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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

const char *disass(BYTE opcode)
{
    if (mnemonic[opcode] == NULL) return "";
    return mnemonic[opcode];
}

BYTE toass(char *token)
{
    int i;

    for (i = 0; i < 0x5c; i++)
	if (strcmp(token, mnemonic[i]) == 0) return i;

    if (strcmp(token, mnemonic[0xff]) == 0) return 0xff;

    return 0xfe;
}

CELL val_EP(void)
{
    return ((BYTE *)EP - M0);
}

char *val_data_stack(void)
{
    CELL *i;
    static char picture[1024];
    char item[16];

    picture[0] = '\0';
    for (i = S0 - 1; i >= SP; i--) {
        sprintf(item, "%ld", *i);
        strcat(picture, item);
        if (i != SP) strcat(picture, " ");
    }

    return picture;
}

void show_data_stack(void)
{
    CELL *i;

    printf("Data stack: ");
    for (i = S0 - 1; i >= SP; i--) printf("%ld ", *i);
    putchar('\n');
}

void show_return_stack(void)
{
    CELL *i;

    printf("Return stack: ");
    for (i = R0 - 1; i >= RP; i--) printf("%lxh ", *i);
    putchar('\n');
}
