/* STEP.C

    (c) Reuben Thomas 1994-2018

    The interface call single_step() : integer.

*/


#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "binary-io.h"

#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */


/* Assumption for file functions */
verify(sizeof(int) <= sizeof(CELL));

/* Address checking */

#define SET_NOT_ADDRESS(a)                      \
    M0[3] = NOT_ADDRESS = (a);

#define CHECK_ADDRESS(a, cond, code, label)     \
    if (!(cond)) {                              \
        SET_NOT_ADDRESS(a);                     \
        exception = code;                       \
        goto label;                             \
    }
#define CHECK_ALIGNED(a)                                \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, badadr)

#define CHECK_MAIN_MEMORY_ALIGNED(a)                    \
    CHECK_ADDRESS(a, IN_MAIN_MEMORY(a), -9, badadr)     \
    CHECK_ALIGNED(a)
#define CHECKP(p)                                       \
    CHECK_MAIN_MEMORY_ALIGNED((BYTE *)(p) - (BYTE *)M0)

#define CHECK_NATIVE_ADDRESS(a, ptr)                            \
    CHECK_ADDRESS(a, ptr = native_address(a), -9, badadr)
#define CHECK_NATIVE_ADDRESS_ALIGNED(a, ptr)    \
    CHECK_NATIVE_ADDRESS(a, ptr)                \
    CHECK_ALIGNED(a)

#define NEXTC                                   \
    CHECKP(EP);                                 \
    NEXT

#define DIVZERO(x)                              \
    if (x == 0) {                               \
        CHECKP(SP - 1);                         \
        *--SP = -10;                            \
        goto throw;                             \
    }


/* LIB support */

#define MAX_LIB_ROUTINE 13

/* Copy a string from Beetle to C */
static char *getstr(UCELL adr, UCELL len)
{
    int exception = 0; // FIXME: use this!

    char *s = calloc(1, len + 1);
    if (!s)
        return NULL;
    for (size_t i = 0; i < len; i++, adr++) {
        unsigned char *ptr;
        CHECK_NATIVE_ADDRESS(adr, ptr);
        s[i] = *(char *)ptr;
    }
    return s;

 badadr:
    return NULL;
}

/* Convert portable open(2) flags bits to system flags */
static int getflags(UCELL perm, bool *binary)
{
    int flags = 0;

    switch (perm & 3) {
    case 0:
        flags = O_RDONLY;
        break;
    case 1:
        flags = O_WRONLY;
        break;
    case 2:
        flags = O_RDWR;
        break;
    default:
        break;
    }
    if (perm & 4)
        flags |= O_CREAT;

    if (perm & 8)
        *binary = true;

    return flags;
}

/* Register command-line args in Beetle high memory */
int main_argc = 0;
UCELL *main_argv;
UCELL *main_argv_len;
bool register_args(int argc, char *argv[])
{
    main_argc = argc;
    if ((main_argv = calloc(argc, sizeof(UCELL))) == NULL ||
        (main_argv_len = calloc(argc, sizeof(UCELL))) == NULL)
        return false;

    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        main_argv[i] = himem_allot(argv[i], len);
        if (main_argv[i] == 0)
            return false;
        main_argv_len[i] = len;
    }
    return true;
}


/* Perform one pass of the execution cycle. */
CELL single_step(void)
{
    int exception = 0;
    CELL temp;
    BYTE *ptr;

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
        for (int i = *SP; i > 0; i--)
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
        temp = FMOD(*(SP + 1), *SP, temp);
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
        *(SP - 1) < (CELL)CELL_BIT ? (*SP <<= *(SP - 1)) : (*SP = 0);
        break;
    case O_RSHIFT:
        CHECKP(SP);
        CHECKP(SP + 1);
        SP++;
        *(SP - 1) < (CELL)CELL_BIT ? (*SP = (CELL)((UCELL)(*SP) >> *(SP - 1))) : (*SP = 0);
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
        CHECK_NATIVE_ADDRESS_ALIGNED(*SP, ptr);
        *SP = *(CELL *)ptr;
        break;
    case O_STORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECK_NATIVE_ADDRESS_ALIGNED(*SP, ptr);
        *(CELL *)ptr = *(SP + 1);
        SP += 2;
        break;
    case O_CFETCH:
        CHECKP(SP);
        CHECK_NATIVE_ADDRESS(*SP, ptr);
        *SP = (CELL)*ptr;
        break;
    case O_CSTORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECK_NATIVE_ADDRESS(*SP, ptr);
        *ptr = (BYTE)*(SP + 1);
        SP += 2;
        break;
    case O_PSTORE:
        CHECKP(SP);
        CHECKP(SP + 1);
        CHECK_NATIVE_ADDRESS_ALIGNED(*SP, ptr);
        *(CELL *)ptr += *(SP + 1);
        SP += 2;
        break;
    case O_SPFETCH:
        CHECKP(SP - 1);
        SP--;
        *SP = (CELL)(SP - M0 + 1) * CELL_W;
        break;
    case O_SPSTORE:
        CHECKP(SP);
        CHECK_MAIN_MEMORY_ALIGNED(*SP);
        SP = *SP / CELL_W + M0;
        break;
    case O_RPFETCH:
        CHECKP(SP - 1);
        *--SP = (CELL)(RP - M0) * CELL_W;
        break;
    case O_RPSTORE:
        CHECKP(SP);
        CHECK_MAIN_MEMORY_ALIGNED(*SP);
        RP = *SP++ / CELL_W + M0;
        break;
    case O_BRANCH:
        CHECKP(EP);
        EP = (CELL *)(*EP + (BYTE *)M0);
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
            EP = (CELL *)(*EP + (BYTE *)M0);
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
        *--RP = (CELL)(EP - M0) * CELL_W;
        EP = (CELL *)(*SP++ + (BYTE *)M0);
        NEXTC;
        break;
    case O_FEXECUTE:
        CHECKP(RP - 1);
        CHECKP(SP);
        CHECKP(*SP + (BYTE *)M0);
        *--RP = (CELL)(EP - M0) * CELL_W;
        EP = (CELL *)(*(CELL *)(*SP++ + (BYTE *)M0) + (BYTE *)M0);
        NEXTC;
        break;
    case O_CALL:
        CHECKP(RP - 1);
        CHECKP(EP);
        *--RP = (CELL)(EP - M0 + 1) * CELL_W;
        EP = (CELL *)(*EP + (BYTE *)M0);
        NEXTC;
        break;
    case O_CALLI:
        CHECKP(RP - 1);
        *--RP = (CELL)(EP - M0) * CELL_W;
        EP += A;
        NEXTC;
        break;
    case O_EXIT:
        CHECKP(RP);
        EP = (CELL *)(*RP++ + (BYTE *)M0);
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
            EP = (CELL *)(*EP + (BYTE *)M0);
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
            EP = (CELL *)(*EP + (BYTE *)M0);
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
        /* EP can be unaligned in an error condition, so do BYTE * arithmetic */
        M0[2] = BAD = (CELL)((BYTE *)EP - (BYTE *)M0);
        if (!IN_MAIN_MEMORY((UCELL)*THROW) || !IS_ALIGNED((UCELL)*THROW))
            return -259;
        EP = (CELL *)((UCELL)*THROW + (BYTE *)M0);
        NEXTC;
        exception = 0; // Any exception has now been dealt with
        break;
    case O_HALT:
        CHECKP(SP);
        return (*SP++);
    case O_EPFETCH:
        CHECKP(SP - 1);
        *--SP = (CELL)(EP - M0) * CELL_W;
        break;
    case O_LIB:
        CHECKP(SP);
        if ((UCELL)(*SP) > MAX_LIB_ROUTINE) {
            CHECKP(SP - 1);
            *--SP = -257;
            goto throw;
        } else {
            switch ((UCELL)*SP++) {
            case 0: /* ARGC ( -- u ) */
                CHECKP(SP - 1);
                *--SP = main_argc;
                break;

            case 1: /* ARG ( u1 -- c-addr u2 )*/
                {
                    CHECKP(SP);
                    UCELL u = *(UCELL *)SP;
                    CHECKP(SP - 1);
                    if (u >= (UCELL)main_argc) {
                        *SP = 0;
                        *--SP = 0;
                    } else {
                        *SP = main_argv[u];
                        *--SP = main_argv_len[u];
                    }
                }
                break;

            case 2: /* STDIN */
                CHECKP(SP - 1);
                *--SP = (CELL)(STDIN_FILENO);
                break;

            case 3: /* STDOUT */
                CHECKP(SP - 1);
                *--SP = (CELL)(STDOUT_FILENO);
                break;

            case 4: /* STDERR */
                CHECKP(SP - 1);
                *--SP = (CELL)(STDERR_FILENO);
                break;

            case 5: /* OPEN-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);
                    CHECKP(SP + 2);
                    char *file = getstr(*((UCELL *)SP + 2), *((UCELL *)SP + 1));
                    bool binary = false;
                    int perm = getflags(*(UCELL *)SP, &binary);
                    int fd = file ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
                    free(file);
                    *++SP = fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0;
                    *(SP + 1) = (CELL)fd;
                }
                break;

            case 6: /* CLOSE-FILE */
                {
                    CHECKP(SP);
                    int fd = *SP;
                    *SP = close(fd);
                }
                break;

            case 7: /* READ-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);
                    CHECKP(SP + 2);

                    int fd = *SP;
                    ssize_t res = 1;
                    UCELL i;
                    for (i = 0; i < *((UCELL *)SP + 1); i++)
                        if ((res = read(fd, (BYTE *)M0 + FLIP(*((UCELL *)SP + 2) + i), 1)) != 1)
                            break;
                    *++SP = res >= 0 ? 0 : -1;
                    *((UCELL *)SP + 1) = i;
                }
                break;

            case 8: /* WRITE-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);
                    CHECKP(SP + 2);

                    int fd = *SP;
                    ssize_t res = 1;
                    for (UCELL i = 0; i < *((UCELL *)SP + 1); i++)
                        if ((res = write(fd, (BYTE *)M0 + FLIP(*((UCELL *)SP + 2) + i), 1)) != 1)
                            break;
                    *(SP += 2) = res >= 0 ? 0 : -1;
                }
                break;

            case 9: /* FILE-POSITION */
                {
                    CHECKP(SP);
                    CHECKP(SP - 1);
                    CHECKP(SP - 2);

                    int fd = *SP;
                    off_t res = lseek(fd, 0, SEEK_CUR);
                    DUCELL ud = res;
                    *((UCELL *)SP--) = (UCELL)(ud & CELL_MASK);
                    *((UCELL *)SP--) = (UCELL)((ud >> CELL_BIT) & CELL_MASK);
                    *SP = res >= 0 ? 0 : -1;
                }
                break;

            case 10: /* REPOSITION-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);
                    CHECKP(SP + 2);

                    int fd = *SP;
                    DUCELL ud = *((UCELL *)SP + 2) | ((DUCELL)*((UCELL *)SP + 1) << CELL_BIT);
                    off_t res = lseek(fd, (off_t)ud, SEEK_SET);
                    *(SP += 2) = res >= 0 ? 0 : -1;
                }
                break;

            case 11: /* FLUSH-FILE */
                {
                    CHECKP(SP);
                    int fd = *SP;
                    int res = fdatasync(fd);
                    *SP = res == 0 ? 0 : -1;
                }
                break;

            case 12: /* RENAME-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);
                    CHECKP(SP + 2);
                    CHECKP(SP + 3);

                    char *from = getstr(*((UCELL *)SP + 3), *((UCELL *)SP + 2));
                    char *to = getstr(*((UCELL *)SP + 1), *(UCELL *)SP);
                    int res = rename(from, to);
                    free(from);
                    free(to);

                    *(SP += 3) = res ? -1 : 0;
                }
                break;

            case 13: /* DELETE-FILE */
                {
                    CHECKP(SP);
                    CHECKP(SP + 1);

                    char *file = getstr(*((UCELL *)SP + 1), *(UCELL *)SP);
                    int res = remove(file);
                    free(file);

                    *++SP = res ? -1 : 0;
                }
                break;

            default: /* Can't happen */
                break;
            }
        }
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
    if (exception == 0)
        return -260; /* terminated OK */

    /* Deal with address exceptions during execution cycle. */
 badadr:
    SP--;
    if ((UCELL)(SP - M0) * CELL_W >= MEMORY || (size_t)SP & (CELL_W - 1))
      return -258;
    *SP = exception;
    goto throw;
}
