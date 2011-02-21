/* STEP.C

    (c) Reuben Thomas 1994-2011

    The interface call single_step() : integer.

*/


#include <stdio.h>
#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


#define CHECKC(a)                                                       \
    if ((UCELL)((BYTE *)(a) - M0) >= MEMORY) {                          \
        *(CELL *)(M0 + 12) = ADDRESS = (BYTE *)(a) - M0;                \
        goto invadr;                                                    \
    }
#define CHECKA(a)                                                      \
    CHECKC(a);                                                         \
    if (((BYTE *)(a) - M0) & 3) {                                      \
        *(CELL *)(M0 + 12) = ADDRESS = (BYTE *)(a) - M0;               \
        goto aliadr;                                                   \
    }
#define NEXTC                                   \
    CHECKA(EP);                                 \
    NEXT

#define DIVZERO(x)                                      \
    if (x == 0) {                                       \
        CHECKA(SP - 1);                                 \
        *--SP = -10;                                    \
        goto throw;                                     \
    }

/* Perform one pass of the execution cycle. */
CELL single_step(void)
{
    CELL temp, i;

    I = (BYTE)A;
    ARSHIFT(A, 8);
    switch (I) {
    case O_NEXT00:
    case O_NEXTFF:
        NEXTC;
        break;
    case O_DUP:
        CHECKA(SP - 1);
        CHECKA(SP);
        SP--;
        *SP = *(SP + 1);
        break;
    case O_DROP:
        SP++;
        break;
    case O_SWAP:
        CHECKA(SP);
        CHECKA(SP + 1);
        temp = *SP;
        *SP = *(SP + 1);
        *(SP + 1) = temp;
        break;
    case O_OVER:
        CHECKA(SP - 1);
        CHECKA(SP + 1);
        SP--;
        *SP = *(SP + 2);
        break;
    case O_ROT:
        CHECKA(SP);
        CHECKA(SP + 1);
        CHECKA(SP + 2);
        temp = *(SP + 2);
        *(SP + 2) = *(SP + 1);
        *(SP + 1) = *SP;
        *SP = temp;
        break;
    case O_NROT:
        CHECKA(SP);
        CHECKA(SP + 1);
        CHECKA(SP + 2);
        temp = *SP;
        *SP = *(SP + 1);
        *(SP + 1) = *(SP + 2);
        *(SP + 2) = temp;
        break;
    case O_TUCK:
        CHECKA(SP - 1);
        CHECKA(SP);
        CHECKA(SP + 1);
        SP--;
        *SP = *(SP + 1);
        *(SP + 1) = *(SP + 2);
        *(SP + 2) = *SP;
        break;
    case O_NIP:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = *(SP - 1);
        break;
    case O_PICK:
        CHECKA(SP);
        CHECKA(SP + *SP + 1);
        *SP = *(SP + *SP + 1);
        break;
    case O_ROLL:
        CHECKA(SP);
        CHECKA(SP + *SP + 1);
        temp = *(SP + *SP + 1);
        for (i = *SP; i > 0; i--)
            *(SP + i + 1) = *(SP + i);
        *++SP = temp;
        break;
    case O_QDUP:
        CHECKA(SP - 1);
        CHECKA(SP);
        if (*SP != 0) {
            SP--;
            *SP = *(SP + 1);
        }
        break;
    case O_TOR:
        CHECKA(RP - 1);
        CHECKA(SP);
        *--RP = *SP++;
        break;
    case O_RFROM:
        CHECKA(SP - 1);
        CHECKA(RP);
        *--SP = *RP++;
        break;
    case O_RFETCH:
        CHECKA(SP - 1);
        CHECKA(RP);
        *--SP = *RP;
        break;
    case O_LESS:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*SP < *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_GREATER:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*SP > *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_EQUAL:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*SP == *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_NEQUAL:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*SP != *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_LESS0:
        CHECKA(SP);
        *SP = (*SP < 0 ? B_TRUE : B_FALSE);
        break;
    case O_GREATER0:
        CHECKA(SP);
        *SP = (*SP > 0 ? B_TRUE : B_FALSE);
        break;
    case O_EQUAL0:
        CHECKA(SP);
        *SP = (*SP == 0 ? B_TRUE : B_FALSE);
        break;
    case O_NEQUAL0:
        CHECKA(SP);
        *SP = (*SP != 0 ? B_TRUE : B_FALSE);
        break;
    case O_ULESS:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = ((UCELL)*SP < (UCELL)*(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_UGREATER:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = ((UCELL)*SP > (UCELL)*(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_ZERO:
        CHECKA(SP - 1);
        *--SP = 0;
        break;
    case O_ONE:
        CHECKA(SP - 1);
        *--SP = 1;
        break;
    case O_MONE:
        CHECKA(SP - 1);
        *--SP = -1;
        break;
    case O_CELL:
        CHECKA(SP - 1);
        *--SP = CELL_W;
        break;
    case O_MCELL:
        CHECKA(SP - 1);
        *--SP = -CELL_W;
        break;
    case O_PLUS:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP += *(SP - 1);
        break;
    case O_MINUS:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP -= *(SP - 1);
        break;
    case O_SWAPMINUS:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = *(SP - 1) - *SP;
        break;
    case O_PLUS1:
        CHECKA(SP);
        *SP += 1;
        break;
    case O_MINUS1:
        CHECKA(SP);
        *SP -= 1;
        break;
    case O_PLUSCELL:
        CHECKA(SP);
        *SP += CELL_W;
        break;
    case O_MINUSCELL:
        CHECKA(SP);
        *SP -= CELL_W;
        break;
    case O_STAR:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP *= *(SP - 1);
        break;
    case O_SLASH:
        CHECKA(SP);
        CHECKA(SP + 1);
        DIVZERO(*SP);
        SP++;
        *SP = DIV(*SP, *(SP - 1));
        break;
    case O_MOD:
        CHECKA(SP);
        CHECKA(SP + 1);
        DIVZERO(*SP);
        SP++;
        *SP = MOD(*SP, *(SP - 1), temp);
        break;
    case O_SLASHMOD:
        CHECKA(SP);
        CHECKA(SP + 1);
        DIVZERO(*SP);
        temp = MOD(*(SP + 1), *SP, i);
        *SP = DIV(*(SP + 1), *SP);
        *(SP + 1) = temp;
        break;
    case O_USLASHMOD:
        CHECKA(SP);
        CHECKA(SP + 1);
        DIVZERO(*SP);
        temp = (UCELL)*(SP + 1) % (UCELL)*SP;
        *SP = (UCELL)*(SP + 1) / (UCELL)*SP;
        *(SP + 1) = (UCELL)temp;
        break;
    case O_SSLASHREM:
        CHECKA(SP);
        CHECKA(SP + 1);
        DIVZERO(*SP);
        temp = SMOD(*(SP + 1), *SP, i);
        *SP = SDIV(*(SP + 1), *SP);
        *(SP + 1) = temp;
        break;
    case O_SLASH2:
        CHECKA(SP);
        ARSHIFT(*SP, 1);
        break;
    case O_CELLS:
        CHECKA(SP);
        *SP *= CELL_W;
        break;
    case O_ABS:
        CHECKA(SP);
        if (*SP < 0)
            *SP = -*SP;
        break;
    case O_NEGATE:
        CHECKA(SP);
        *SP = -*SP;
        break;
    case O_MAX:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*(SP - 1) > *SP ? *(SP - 1) : *SP);
        break;
    case O_MIN:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP = (*(SP - 1) < *SP ? *(SP - 1) : *SP);
        break;
    case O_INVERT:
        CHECKA(SP);
        *SP = ~*SP;
        break;
    case O_AND:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP &= *(SP - 1);
        break;
    case O_OR:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP |= *(SP - 1);
        break;
    case O_XOR:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *SP ^= *(SP - 1);
        break;
    case O_LSHIFT:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *(SP - 1) < 32 ? (*SP <<= *(SP - 1)) : (*SP = 0);
        break;
    case O_RSHIFT:
        CHECKA(SP);
        CHECKA(SP + 1);
        SP++;
        *(SP - 1) < 32 ? (*SP = (CELL)((UCELL)(*SP) >> *(SP - 1))) : (*SP = 0);
        break;
    case O_LSHIFT1:
        CHECKA(SP);
        *SP <<= 1;
        break;
    case O_RSHIFT1:
        CHECKA(SP);
        *SP = (CELL)((UCELL)(*SP) >> 1);
        break;
    case O_FETCH:
        CHECKA(SP);
        CHECKA(*SP + M0);
        *SP = *(CELL *)(*SP + M0);
        break;
    case O_STORE:
        CHECKA(SP);
        CHECKA(SP + 1);
        CHECKA(*SP + M0);
        *(CELL *)(*SP + M0) = *(SP + 1);
        SP += 2;
        break;
    case O_CFETCH:
        CHECKA(SP);
        CHECKC(FLIP(*SP) + M0);
        *SP = (CELL)*(FLIP(*SP) + M0);
        break;
    case O_CSTORE:
        CHECKA(SP);
        CHECKA(SP + 1);
        CHECKC(FLIP(*SP) + M0);
        *(FLIP(*SP) + M0) = (BYTE)*(SP + 1);
        SP += 2;
        break;
    case O_PSTORE:
        CHECKA(SP);
        CHECKA(SP + 1);
        CHECKA(*SP + M0);
        *(CELL *)(*SP + M0) += *(SP + 1);
        SP += 2;
        break;
    case O_SPFETCH:
        CHECKA(SP - 1);
        SP--;
        *SP = (CELL)((BYTE *)SP - M0) + CELL_W;
        break;
    case O_SPSTORE:
        CHECKA(SP);
        SP = (CELL *)(*SP + M0);
        break;
    case O_RPFETCH:
        CHECKA(SP - 1);
        *--SP = (CELL)((BYTE *)RP - M0);
        break;
    case O_RPSTORE:
        CHECKA(SP);
        RP = (CELL *)(*SP++ + M0);
        break;
    case O_BRANCH:
        CHECKA(EP);
        EP = (CELL *)(*EP + M0);
        NEXTC;
        break;
    case O_BRANCHI:
        EP += A;
        NEXTC;
        break;
    case O_QBRANCH:
        CHECKA(SP);
        CHECKA(EP);
        if (*SP++ == B_FALSE) {
            EP = (CELL *)(*EP + M0);
            NEXTC;
        } else
            EP++;
        break;
    case O_QBRANCHI:
        CHECKA(SP);
        if (*SP++ == B_FALSE)
            EP += A;
        NEXTC;
        break;
    case O_EXECUTE:
        CHECKA(RP - 1);
        CHECKA(SP);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP = (CELL *)(*SP++ + M0);
        NEXTC;
        break;
    case O_FEXECUTE:
        CHECKA(RP - 1);
        CHECKA(SP);
        CHECKA(*SP + M0);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP = (CELL *)(*(CELL *)(*SP++ + M0) + M0);
        NEXTC;
        break;
    case O_CALL:
        CHECKA(RP - 1);
        CHECKA(EP);
        *--RP = (CELL)((BYTE *)EP - M0) + CELL_W;
        EP = (CELL *)(*EP + M0);
        NEXTC;
        break;
    case O_CALLI:
        CHECKA(RP - 1);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP += A;
        NEXTC;
        break;
    case O_EXIT:
        CHECKA(RP);
        EP = (CELL *)(*RP++ + M0);
        NEXTC;
        break;
    case O_DO:
        CHECKA(RP - 1);
        CHECKA(RP - 2);
        CHECKA(SP);
        CHECKA(SP + 1);
        *--RP = *(SP + 1);
        *--RP = *SP++;
        SP++;
        break;
    case O_LOOP:
        CHECKA(RP);
        CHECKA(RP + 1);
        CHECKA(EP);
        (*RP)++;
        if (*RP == *(RP + 1)) {
            RP += 2;
            EP++;
        } else {
            EP = (CELL *)(*EP + M0);
            NEXTC;
        }
        break;
    case O_LOOPI:
        CHECKA(RP);
        CHECKA(RP + 1);
        (*RP)++;
        if (*RP == *(RP + 1))
            RP += 2;
        else
            EP += A;
        NEXTC;
        break;
    case O_PLOOP:
        CHECKA(RP);
        CHECKA(RP + 1);
        CHECKA(SP);
        CHECKA(EP);
        temp = *RP - *(RP + 1);
        *RP += *SP++;
        if (((*RP - *(RP + 1)) ^ temp) < 0) {
            RP += 2;
            EP++;
        } else {
            EP = (CELL *)(*EP + M0);
            NEXTC;
        } break;
    case O_PLOOPI:
        CHECKA(RP);
        CHECKA(RP + 1);
        CHECKA(SP);
        temp = *RP - *(RP + 1);
        *RP += *SP++;
        if (((*RP - *(RP + 1)) ^ temp) < 0)
            RP += 2;
        else
            EP += A;
        NEXTC;
        break;
    case O_UNLOOP:
        RP += 2;
        break;
    case O_J:
        CHECKA(SP - 1);
        CHECKA(RP + 2);
        *--SP = *(RP + 2);
        break;
    case O_LITERAL:
        CHECKA(SP - 1);
        CHECKA(EP);
        *--SP = *EP++;
        break;
    case O_LITERALI:
        CHECKA(SP - 1);
        *--SP = A;
        NEXTC;
        break;
 throw:
    case O_THROW:
        *(CELL *)(M0 + 8) = BAD = (CELL)((BYTE *)EP - M0);
        temp = (UCELL)*THROW;
        if ((UCELL)temp >= MEMORY || (unsigned int)temp & 3)
            return -259;
        EP = (CELL *)(temp + M0);
        NEXTC;
        break;
    case O_HALT:
        CHECKA(SP);
        return (*SP++);
    case O_CREATE:
        CHECKA(SP - 1);
        *--SP = (CELL)((BYTE *)EP - M0);
        break;
    case O_LIB:
        CHECKA(SP);
        CHECKA(SP - 1);
        if ((UCELL)(*SP) > 12) {
            *--SP = -257;
            goto throw;
        } else
            lib((UCELL)*SP++);
        break;
    case O_OS:
        break;
    case O_LINK:
        CHECKA(SP);
        LINK;
        break;
    case O_RUN:
        break;
    case O_STEP:
        break;
    default:
        CHECKA(SP - 1);
        *--SP = -256;
        goto throw;
    }
    return -260; /* terminated OK */

    /* Deal with address exceptions during execution cycle. */
 invadr:
    SP--;
    if ((UCELL)((BYTE *)SP - M0) >= MEMORY * CELL_W || (unsigned int)SP & 3)
      return -258;
    *SP = -9;
    goto throw;
 aliadr:
    SP--;
    if ((UCELL)((BYTE *)SP - M0) >= MEMORY * CELL_W || (unsigned int)SP & 3)
      return -258;
    *SP = -23;
    goto throw;
}
