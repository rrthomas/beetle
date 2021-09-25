// enum type for the opcodes to make the interpreter more readable. Opcode
// names which are not valid C identifiers have been altered.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEETLE_OPCODES
#define BEETLE_OPCODES


enum {
    O_NEXT00,
    O_DUP,
    O_DROP,
    O_SWAP,
    O_OVER,
    O_ROT,
    O_NROT,
    O_TUCK,
    O_NIP,
    O_PICK,
    O_ROLL,
    O_QDUP,
    O_TOR,
    O_RFROM,
    O_RFETCH,
    O_LESS,
    O_GREATER,
    O_EQUAL,
    O_NEQUAL,
    O_LESS0,
    O_GREATER0,
    O_EQUAL0,
    O_NEQUAL0,
    O_ULESS,
    O_UGREATER,
    O_ZERO,
    O_ONE,
    O_MONE,
    O_CELL,
    O_MCELL,
    O_PLUS,
    O_MINUS,
    O_SWAPMINUS,
    O_PLUS1,
    O_MINUS1,
    O_PLUSCELL,
    O_MINUSCELL,
    O_STAR,
    O_SLASH,
    O_MOD,
    O_SLASHMOD,
    O_USLASHMOD,
    O_SSLASHREM,
    O_SLASH2,
    O_CELLS,
    O_ABS,
    O_NEGATE,
    O_MAX,
    O_MIN,
    O_INVERT,
    O_AND,
    O_OR,
    O_XOR,
    O_LSHIFT,
    O_RSHIFT,
    O_LSHIFT1,
    O_RSHIFT1,
    O_FETCH,
    O_STORE,
    O_CFETCH,
    O_CSTORE,
    O_PSTORE,
    O_SPFETCH,
    O_SPSTORE,
    O_RPFETCH,
    O_RPSTORE,
    O_BRANCH,
    O_BRANCHI,
    O_QBRANCH,
    O_QBRANCHI,
    O_EXECUTE,
    O_FEXECUTE,
    O_CALL,
    O_CALLI,
    O_EXIT,
    O_DO,
    O_LOOP,
    O_LOOPI,
    O_PLOOP,
    O_PLOOPI,
    O_UNLOOP,
    O_J,
    O_LITERAL,
    O_LITERALI,
    O_THROW,
    O_HALT,
    O_EPFETCH,
    O_LIB,
    O_UNDEFINED,
    O_LINK,
    O_S0FETCH,
    O_S0STORE,
    O_R0FETCH,
    O_R0STORE,
    O_THROWFETCH,
    O_THROWSTORE,
    O_MEMORYFETCH,
    O_BADFETCH,
    O_NOT_ADDRESSFETCH,
    O_NEXTFF = 0xff
};


#endif
