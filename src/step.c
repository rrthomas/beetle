/* STEP.C

    (c) Reuben Thomas 1994-2018

    The interface call single_step() : integer.

*/


#include <stdio.h>
#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* lib function */


#define CHECKC(a)                               \
    if ((UCELL)(a) >= MEMORY) {                 \
        *(CELL *)(M0 + 12) = ADDRESS = (a);     \
        goto invadr;                            \
    }
#define CHECKA(a)                               \
    CHECKC(a);                                  \
    if ((a) & 3) {                              \
        *(CELL *)(M0 + 12) = ADDRESS = (a);     \
        goto aliadr;                            \
    }
#define CHECKP(p)                               \
    CHECKA((BYTE *)(p) - M0)
#define NEXTC                                   \
    CHECKP(EP);                                 \
    NEXT

#define DIVZERO(x)                              \
    if (x == 0) {                               \
        CHECKP(SP - 1);                         \
        *--SP = -10;                            \
        goto throw;                             \
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
        CHECKP(SP - 1);
        CHECKP(SP);
        SP--;
        *SP = *(SP + 1);
        break;
    case O_DROP:
        SP++;
        break;
    case O_SWAP:
        CHECKP(SP);
        CHECKP(SP + 1);
        temp = *SP;
        *SP = *(SP + 1);
        *(SP + 1) = temp;
        break;
    case O_OVER:
        CHECKP(SP - 1);
        CHECKP(SP + 1);
        SP--;
        *SP = *(SP + 2);
        break;
    case O_ROT:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECKP(SP + 2);
        temp = *(SP + 2);
        *(SP + 2) = *(SP + 1);
        *(SP + 1) = *SP;
        *SP = temp;
        break;
    case O_NROT:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECKP(SP + 2);
        temp = *SP;
        *SP = *(SP + 1);
        *(SP + 1) = *(SP + 2);
        *(SP + 2) = temp;
        break;
    case O_TUCK:
        CHECKP(SP - 1);
        CHECKP(SP);
        CHECKP(SP + 1);
        SP--;
        *SP = *(SP + 1);
        *(SP + 1) = *(SP + 2);
        *(SP + 2) = *SP;
        break;
    case O_NIP:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = *(SP - 1);
        break;
    case O_PICK:
        CHECKP(SP);
        CHECKP(SP + *SP + 1);
        *SP = *(SP + *SP + 1);
        break;
    case O_ROLL:
        CHECKP(SP);
        CHECKP(SP + *SP + 1);
        temp = *(SP + *SP + 1);
        for (i = *SP; i > 0; i--)
            *(SP + i + 1) = *(SP + i);
        *++SP = temp;
        break;
    case O_QDUP:
        CHECKP(SP - 1);
        CHECKP(SP);
        if (*SP != 0) {
            SP--;
            *SP = *(SP + 1);
        }
        break;
    case O_TOR:
        CHECKP(RP - 1);
        CHECKP(SP);
        *--RP = *SP++;
        break;
    case O_RFROM:
        CHECKP(SP - 1);
        CHECKP(RP);
        *--SP = *RP++;
        break;
    case O_RFETCH:
        CHECKP(SP - 1);
        CHECKP(RP);
        *--SP = *RP;
        break;
    case O_LESS:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*SP < *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_GREATER:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*SP > *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_EQUAL:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*SP == *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_NEQUAL:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*SP != *(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_LESS0:
        CHECKP(SP);
        *SP = (*SP < 0 ? B_TRUE : B_FALSE);
        break;
    case O_GREATER0:
        CHECKP(SP);
        *SP = (*SP > 0 ? B_TRUE : B_FALSE);
        break;
    case O_EQUAL0:
        CHECKP(SP);
        *SP = (*SP == 0 ? B_TRUE : B_FALSE);
        break;
    case O_NEQUAL0:
        CHECKP(SP);
        *SP = (*SP != 0 ? B_TRUE : B_FALSE);
        break;
    case O_ULESS:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = ((UCELL)*SP < (UCELL)*(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_UGREATER:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = ((UCELL)*SP > (UCELL)*(SP - 1) ? B_TRUE : B_FALSE);
        break;
    case O_ZERO:
        CHECKP(SP - 1);
        *--SP = 0;
        break;
    case O_ONE:
        CHECKP(SP - 1);
        *--SP = 1;
        break;
    case O_MONE:
        CHECKP(SP - 1);
        *--SP = -1;
        break;
    case O_CELL:
        CHECKP(SP - 1);
        *--SP = CELL_W;
        break;
    case O_MCELL:
        CHECKP(SP - 1);
        *--SP = -CELL_W;
        break;
    case O_PLUS:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(UCELL *)SP += (UCELL)*(SP - 1);
        break;
    case O_MINUS:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(UCELL *)SP -= (UCELL)*(SP - 1);
        break;
    case O_SWAPMINUS:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(UCELL *)SP = (UCELL)*(SP - 1) - (UCELL)*SP;
        break;
    case O_PLUS1:
        CHECKP(SP);
        *(UCELL *)SP += 1;
        break;
    case O_MINUS1:
        CHECKP(SP);
        *(UCELL *)SP -= 1;
        break;
    case O_PLUSCELL:
        CHECKP(SP);
        *(UCELL *)SP += CELL_W;
        break;
    case O_MINUSCELL:
        CHECKP(SP);
        *(UCELL *)SP -= CELL_W;
        break;
    case O_STAR:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(UCELL *)SP *= (UCELL)*(SP - 1);
        break;
    case O_SLASH:
        CHECKP(SP);
        CHECKP(SP + 1);
        DIVZERO(*SP);
        SP++;
        *SP = FDIV(*SP, *(SP - 1));
        break;
    case O_MOD:
        CHECKP(SP);
        CHECKP(SP + 1);
        DIVZERO(*SP);
        SP++;
        *SP = FMOD(*SP, *(SP - 1), temp);
        break;
    case O_SLASHMOD:
        CHECKP(SP);
        CHECKP(SP + 1);
        DIVZERO(*SP);
        temp = FMOD(*(SP + 1), *SP, i);
        *SP = FDIV(*(SP + 1), *SP);
        *(SP + 1) = temp;
        break;
    case O_USLASHMOD:
        CHECKP(SP);
        CHECKP(SP + 1);
        DIVZERO(*SP);
        temp = (UCELL)*(SP + 1) % (UCELL)*SP;
        *SP = (UCELL)*(SP + 1) / (UCELL)*SP;
        *(SP + 1) = (UCELL)temp;
        break;
    case O_SSLASHREM:
        CHECKP(SP);
        CHECKP(SP + 1);
        DIVZERO(*SP);
        temp = *(SP + 1) % *SP;
        *SP = *(SP + 1) / *SP;
        *(SP + 1) = temp;
        break;
    case O_SLASH2:
        CHECKP(SP);
        ARSHIFT(*SP, 1);
        break;
    case O_CELLS:
        CHECKP(SP);
        *(UCELL *)SP *= CELL_W;
        break;
    case O_ABS:
        CHECKP(SP);
        if (*SP < 0)
            *(UCELL *)SP = -(UCELL)*SP;
        break;
    case O_NEGATE:
        CHECKP(SP);
        *(UCELL *)SP = -(UCELL)*SP;
        break;
    case O_MAX:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*(SP - 1) > *SP ? *(SP - 1) : *SP);
        break;
    case O_MIN:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP = (*(SP - 1) < *SP ? *(SP - 1) : *SP);
        break;
    case O_INVERT:
        CHECKP(SP);
        *SP = ~*SP;
        break;
    case O_AND:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP &= *(SP - 1);
        break;
    case O_OR:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP |= *(SP - 1);
        break;
    case O_XOR:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *SP ^= *(SP - 1);
        break;
    case O_LSHIFT:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(SP - 1) < 32 ? (*SP <<= *(SP - 1)) : (*SP = 0);
        break;
    case O_RSHIFT:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(SP - 1) < 32 ? (*SP = (CELL)((UCELL)(*SP) >> *(SP - 1))) : (*SP = 0);
        break;
    case O_LSHIFT1:
        CHECKP(SP);
        *SP <<= 1;
        break;
    case O_RSHIFT1:
        CHECKP(SP);
        *SP = (CELL)((UCELL)(*SP) >> 1);
        break;
    case O_FETCH:
        CHECKP(SP);
        CHECKA(*SP);
        *SP = *(CELL *)(*SP + M0);
        break;
    case O_STORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECKA(*SP);
        *(CELL *)(*SP + M0) = *(SP + 1);
        SP += 2;
        break;
    case O_CFETCH:
        CHECKP(SP);
        CHECKC(FLIP(*SP));
        *SP = (CELL)*(FLIP(*SP) + M0);
        break;
    case O_CSTORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECKC(FLIP(*SP));
        *(FLIP(*SP) + M0) = (BYTE)*(SP + 1);
        SP += 2;
        break;
    case O_PSTORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECKA(*SP);
        *(CELL *)(*SP + M0) += *(SP + 1);
        SP += 2;
        break;
    case O_SPFETCH:
        CHECKP(SP - 1);
        SP--;
        *SP = (CELL)((BYTE *)SP - M0) + CELL_W;
        break;
    case O_SPSTORE:
        CHECKP(SP);
        SP = (CELL *)(*SP + M0);
        break;
    case O_RPFETCH:
        CHECKP(SP - 1);
        *--SP = (CELL)((BYTE *)RP - M0);
        break;
    case O_RPSTORE:
        CHECKP(SP);
        RP = (CELL *)(*SP++ + M0);
        break;
    case O_BRANCH:
        CHECKP(EP);
        EP = (CELL *)(*EP + M0);
        NEXTC;
        break;
    case O_BRANCHI:
        EP += A;
        NEXTC;
        break;
    case O_QBRANCH:
        CHECKP(SP);
        CHECKP(EP);
        if (*SP++ == B_FALSE) {
            EP = (CELL *)(*EP + M0);
            NEXTC;
        } else
            EP++;
        break;
    case O_QBRANCHI:
        CHECKP(SP);
        if (*SP++ == B_FALSE)
            EP += A;
        NEXTC;
        break;
    case O_EXECUTE:
        CHECKP(RP - 1);
        CHECKP(SP);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP = (CELL *)(*SP++ + M0);
        NEXTC;
        break;
    case O_FEXECUTE:
        CHECKP(RP - 1);
        CHECKP(SP);
        CHECKP(*SP + M0);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP = (CELL *)(*(CELL *)(*SP++ + M0) + M0);
        NEXTC;
        break;
    case O_CALL:
        CHECKP(RP - 1);
        CHECKP(EP);
        *--RP = (CELL)((BYTE *)EP - M0) + CELL_W;
        EP = (CELL *)(*EP + M0);
        NEXTC;
        break;
    case O_CALLI:
        CHECKP(RP - 1);
        *--RP = (CELL)((BYTE *)EP - M0);
        EP += A;
        NEXTC;
        break;
    case O_EXIT:
        CHECKP(RP);
        EP = (CELL *)(*RP++ + M0);
        NEXTC;
        break;
    case O_DO:
        CHECKP(RP - 1);
        CHECKP(RP - 2);
        CHECKP(SP);
        CHECKP(SP + 1);
        *--RP = *(SP + 1);
        *--RP = *SP++;
        SP++;
        break;
    case O_LOOP:
        CHECKP(RP);
        CHECKP(RP + 1);
        CHECKP(EP);
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
        CHECKP(RP);
        CHECKP(RP + 1);
        (*RP)++;
        if (*RP == *(RP + 1))
            RP += 2;
        else
            EP += A;
        NEXTC;
        break;
    case O_PLOOP:
        CHECKP(RP);
        CHECKP(RP + 1);
        CHECKP(SP);
        CHECKP(EP);
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
        CHECKP(RP);
        CHECKP(RP + 1);
        CHECKP(SP);
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
        CHECKP(SP - 1);
        CHECKP(RP + 2);
        *--SP = *(RP + 2);
        break;
    case O_LITERAL:
        CHECKP(SP - 1);
        CHECKP(EP);
        *--SP = *EP++;
        break;
    case O_LITERALI:
        CHECKP(SP - 1);
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
        CHECKP(SP);
        return (*SP++);
    case O_CREATE:
        CHECKP(SP - 1);
        *--SP = (CELL)((BYTE *)EP - M0);
        break;
    case O_LIB:
        CHECKP(SP);
        CHECKP(SP - 1);
        if ((UCELL)(*SP) > 12) {
            *--SP = -257;
            goto throw;
        } else
            lib((UCELL)*SP++);
        break;
    case O_OS:
        break;
    case O_LINK:
        {
            CELL_pointer address;
            int i;
            for (i = POINTER_W - 1; i >= 0; i--) {
                CHECKP(SP);
                address.cells[i] = *SP++;
            }
            address.pointer();
        }
        break;
    case O_RUN:
        break;
    case O_STEP:
        break;
    default:
        CHECKP(SP - 1);
        *--SP = -256;
        goto throw;
    }
    return -260; /* terminated OK */

    /* Deal with address exceptions during execution cycle. */
 invadr:
    SP--;
    if ((UCELL)((BYTE *)SP - M0) >= MEMORY * CELL_W || (size_t)SP & 3)
      return -258;
    *SP = -9;
    goto throw;
 aliadr:
    SP--;
    if ((UCELL)((BYTE *)SP - M0) >= MEMORY * CELL_W || (size_t)SP & 3)
      return -258;
    *SP = -23;
    goto throw;
}
