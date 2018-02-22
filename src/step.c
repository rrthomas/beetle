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
#include "minmax.h"

#include "beetle.h"     /* main header */
#include "opcodes.h"	/* opcode enumeration */


/* Assumption for file functions */
verify(sizeof(int) <= sizeof(CELL));

#define DIVZERO(x)                              \
    if (x == 0) {                               \
        PUSH(-10);                              \
        goto throw;                             \
    }


/* LIB support */

/* Copy a string from Beetle to C */
static int getstr(UCELL adr, UCELL len, char **res)
{
    int exception = 0;

    *res = calloc(1, len + 1);
    if (*res == NULL)
        exception = -261; // FIXME: Document this!
    else
        for (size_t i = 0; exception == 0 && i < len; i++, adr++) {
            exception = beetle_load_byte(adr, (BYTE *)((*res) + i));
        }

    return exception;
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
    CELL temp, temp2;
    BYTE byte;

    I = (BYTE)A;
    ARSHIFT(A, 8);
    switch (I) {
    case O_NEXT00:
    case O_NEXTFF:
        NEXTC;
        break;
    case O_DUP:
        {
            CELL top = LOAD_CELL(SP);
            PUSH(top);
        }
        break;
    case O_DROP:
        (void)POP;
        break;
    case O_SWAP:
        {
            CELL top = POP;
            CELL next = POP;
            PUSH(top);
            PUSH(next);
        }
        break;
    case O_OVER:
        {
            CELL next = LOAD_CELL(SP + CELL_W);
            PUSH(next);
        }
        break;
    case O_ROT:
        {
            CELL top = POP;
            CELL next = POP;
            CELL last = POP;
            PUSH(next);
            PUSH(top);
            PUSH(last);
        }
        break;
    case O_NROT:
        {
            CELL top = POP;
            CELL next = POP;
            CELL last = POP;
            PUSH(top);
            PUSH(last);
            PUSH(next);
        }
        break;
    case O_TUCK:
        {
            CELL top = POP;
            CELL next = POP;
            PUSH(top);
            PUSH(next);
            PUSH(top);
        }
        break;
    case O_NIP:
        {
            CELL keep = POP;
            (void)POP;
            PUSH(keep);
        }
        break;
    case O_PICK:
        {
            CELL depth = POP;
            CELL pickee = LOAD_CELL(SP + depth * CELL_W);
            PUSH(pickee);
        }
        break;
    case O_ROLL:
        {
            CELL depth = POP;
            CELL rollee = LOAD_CELL(SP + depth * CELL_W);
            for (int i = depth; i > 0; i--)
                STORE_CELL(SP + i * CELL_W,
                           LOAD_CELL(SP + (i - 1) * CELL_W));
            STORE_CELL(SP, rollee);
        }
        break;
    case O_QDUP:
        {
            CELL value = LOAD_CELL(SP);
            if (value != 0)
                PUSH(value);
        }
        break;
    case O_TOR:
        {
            CELL value = POP;
            PUSH_RETURN(value);
        }
        break;
    case O_RFROM:
        {
            CELL value = POP_RETURN;
            PUSH(value);
        }
        break;
    case O_RFETCH:
        {
            CELL value = LOAD_CELL(RP);
            PUSH(value);
        }
        break;
    case O_LESS:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(b < a ? B_TRUE : B_FALSE);
        }
        break;
    case O_GREATER:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(b > a ? B_TRUE : B_FALSE);
        }
        break;
    case O_EQUAL:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a == b ? B_TRUE : B_FALSE);
        }
        break;
    case O_NEQUAL:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a != b ? B_TRUE : B_FALSE);
        }
        break;
    case O_LESS0:
        {
            CELL a = POP;
            PUSH(a < 0 ? B_TRUE : B_FALSE);
        }
        break;
    case O_GREATER0:
        {
            CELL a = POP;
            PUSH(a > 0 ? B_TRUE : B_FALSE);
        }
        break;
    case O_EQUAL0:
        {
            CELL a = POP;
            PUSH(a == 0 ? B_TRUE : B_FALSE);
        }
        break;
    case O_NEQUAL0:
        {
            CELL a = POP;
            PUSH(a != 0 ? B_TRUE : B_FALSE);
        }
        break;
    case O_ULESS:
        {
            UCELL a = POP;
            UCELL b = POP;
            PUSH(b < a ? B_TRUE : B_FALSE);
        }
        break;
    case O_UGREATER:
        {
            UCELL a = POP;
            UCELL b = POP;
            PUSH(b > a ? B_TRUE : B_FALSE);
        }
        break;
    case O_ZERO:
        PUSH(0);
        break;
    case O_ONE:
        PUSH(1);
        break;
    case O_MONE:
        PUSH(-1);
        break;
    case O_CELL:
        PUSH(CELL_W);
        break;
    case O_MCELL:
        PUSH(-CELL_W);
        break;
    case O_PLUS:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(b + a);
        }
        break;
    case O_MINUS:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(b - a);
        }
        break;
    case O_SWAPMINUS:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a - b);
        }
        break;
    case O_PLUS1:
        {
            CELL a = POP;
            PUSH(a + 1);
        }
        break;
    case O_MINUS1:
        {
            CELL a = POP;
            PUSH(a - 1);
        }
        break;
    case O_PLUSCELL:
        {
            CELL a = POP;
            PUSH(a + CELL_W);
        }
        break;
    case O_MINUSCELL:
        {
            CELL a = POP;
            PUSH(a - CELL_W);
        }
        break;
    case O_STAR:
        {
            CELL multiplier = POP;
            CELL multiplicand = POP;
            PUSH(multiplier * multiplicand);
        }
        break;
    case O_SLASH:
        {
            CELL divisor = POP;
            CELL dividend = POP;
            DIVZERO(divisor);
            PUSH(FDIV(dividend, divisor));
        }
        break;
    case O_MOD:
        {
            CELL divisor = POP;
            CELL dividend = POP;
            DIVZERO(divisor);
            PUSH(FMOD(dividend, divisor, temp));
        }
        break;
    case O_SLASHMOD:
        {
            CELL divisor = POP;
            CELL dividend = POP;
            DIVZERO(divisor);
            PUSH(FMOD(dividend, divisor, temp));
            PUSH(FDIV(dividend, divisor));
        }
        break;
    case O_USLASHMOD:
        {
            UCELL divisor = POP;
            UCELL dividend = POP;
            DIVZERO(divisor);
            PUSH(dividend % divisor);
            PUSH(dividend / divisor);
        }
        break;
    case O_SSLASHREM:
        {
            CELL divisor = POP;
            CELL dividend = POP;
            DIVZERO(divisor);
            PUSH(dividend % divisor);
            PUSH(dividend / divisor);
        }
        break;
    case O_SLASH2:
        {
            CELL a = POP;
            PUSH(ARSHIFT(a, 1));
        }
        break;
    case O_CELLS:
        {
            CELL a = POP;
            PUSH(a * CELL_W);
        }
        break;
    case O_ABS:
        {
            CELL a = POP;
            PUSH(a < 0 ? -a : a);
        }
        break;
    case O_NEGATE:
        {
            CELL a = POP;
            PUSH(-a);
        }
        break;
    case O_MAX:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(MAX(a, b));
        }
        break;
    case O_MIN:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(MIN(a, b));
        }
        break;
    case O_INVERT:
        {
            CELL a = POP;
            PUSH(~a);
        }
        break;
    case O_AND:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a & b);
        }
        break;
    case O_OR:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a | b);
        }
        break;
    case O_XOR:
        {
            CELL a = POP;
            CELL b = POP;
            PUSH(a ^ b);
        }
        break;
    case O_LSHIFT:
        {
            CELL shift = POP;
            CELL value = POP;
            PUSH(shift < (CELL)CELL_BIT ? value << shift : 0);
        }
        break;
    case O_RSHIFT:
        {
            CELL shift = POP;
            CELL value = POP;
            PUSH(shift < (CELL)CELL_BIT ? (CELL)((UCELL)value >> shift) : 0);
        }
        break;
    case O_LSHIFT1:
        {
            CELL value = POP;
            PUSH(value << 1);
        }
        break;
    case O_RSHIFT1:
        {
            CELL value = POP;
            PUSH((CELL)((UCELL)(value) >> 1));
        }
        break;
    case O_FETCH:
        {
            CELL addr = POP;
            CELL value = LOAD_CELL(addr);
            PUSH(value);
        }
        break;
    case O_STORE:
        {
            CELL addr = POP;
            CELL value = POP;
            STORE_CELL(addr, value);
        }
        break;
    case O_CFETCH:
        {
            CELL addr = POP;
            BYTE value = LOAD_BYTE(addr);
            PUSH((CELL)value);
        }
        break;
    case O_CSTORE:
        {
            CELL addr = POP;
            BYTE value = (BYTE)POP;
            STORE_BYTE(addr, value);
        }
        break;
    case O_PSTORE:
        {
            CELL addr = POP;
            CELL increment = POP;
            CELL value = LOAD_CELL(addr) + increment;
            STORE_CELL(addr, value);
        }
        break;
    case O_SPFETCH:
        {
            CELL value = SP;
            PUSH(value);
        }
        break;
    case O_SPSTORE:
        {
            CELL value = POP;
            CHECK_MAIN_MEMORY_ALIGNED(value);
            SP = value;
        }
        break;
    case O_RPFETCH:
        PUSH(RP);
        break;
    case O_RPSTORE:
        {
            CELL value = POP;
            CHECK_MAIN_MEMORY_ALIGNED(value);
            RP = value;
        }
        break;
    case O_BRANCH:
        {
            CELL addr = LOAD_CELL(EP);
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            EP = addr;
            NEXTC;
        }
        break;
    case O_BRANCHI:
        EP += A * CELL_W;
        NEXTC;
        break;
    case O_QBRANCH:
        if (POP == B_FALSE) {
            CELL addr = LOAD_CELL(EP);
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            EP = addr;
            NEXTC;
        } else
            EP += CELL_W;
        break;
    case O_QBRANCHI:
        if (POP == B_FALSE)
            EP += A * CELL_W;
        NEXTC;
        break;
    case O_EXECUTE:
        {
            CELL addr = POP;
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            PUSH_RETURN(EP);
            EP = addr;
            NEXTC;
        }
        break;
    case O_FEXECUTE:
        {
            CELL addr = POP;
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            PUSH_RETURN(EP);
            addr = LOAD_CELL(addr);
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            EP = addr;
            NEXTC;
        }
        break;
    case O_CALL:
        {
            PUSH_RETURN(EP + CELL_W);
            CELL addr = LOAD_CELL(EP);
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            EP = addr;
            NEXTC;
        }
        break;
    case O_CALLI:
        PUSH_RETURN(EP);
        EP += A * CELL_W;
        NEXTC;
        break;
    case O_EXIT:
        {
            CELL addr = POP_RETURN;
            CHECK_MAIN_MEMORY_ALIGNED(addr);
            EP = addr;
            NEXTC;
        }
        break;
    case O_DO:
        {
            CELL start = POP;
            CELL limit = POP;
            PUSH_RETURN(limit);
            PUSH_RETURN(start);
        }
        break;
    case O_LOOP:
        {
            CELL index = POP_RETURN;
            CELL limit = LOAD_CELL(RP);
            PUSH_RETURN(index + 1);
            if (index + 1 == limit) {
                (void)POP_RETURN;
                (void)POP_RETURN;
                EP += CELL_W;
            } else {
                CELL addr = LOAD_CELL(EP);
                CHECK_MAIN_MEMORY_ALIGNED(addr);
                EP = addr;
                NEXTC;
            }
        }
        break;
    case O_LOOPI:
        {
            CELL index = POP_RETURN;
            CELL limit = LOAD_CELL(RP);
            PUSH_RETURN(index + 1);
            if (index + 1 == limit) {
                (void)POP_RETURN;
                (void)POP_RETURN;
            } else
                EP += A * CELL_W;
            NEXTC;
        }
        break;
    case O_PLOOP:
        {
            CELL index = POP_RETURN;
            CELL limit = LOAD_CELL(RP);
            CELL diff = index - limit;
            CELL inc = POP;
            PUSH_RETURN(index + inc);
            if (((index + inc - limit) ^ diff) < 0) {
                (void)POP_RETURN;
                (void)POP_RETURN;
                EP += CELL_W;
            } else {
                CELL addr = LOAD_CELL(EP);
                CHECK_MAIN_MEMORY_ALIGNED(addr);
                EP = addr;
                NEXTC;
            }
        }
        break;
    case O_PLOOPI:
        {
            CELL index = POP_RETURN;
            CELL limit = LOAD_CELL(RP);
            CELL diff = index - limit;
            CELL inc = POP;
            PUSH_RETURN(index + inc);
            if (((index + inc - limit) ^ diff) < 0) {
                (void)POP_RETURN;
                (void)POP_RETURN;
            } else
                EP += A * CELL_W;
            NEXTC;
        }
        break;
    case O_UNLOOP:
        (void)POP_RETURN;
        (void)POP_RETURN;
        break;
    case O_J:
        PUSH(LOAD_CELL(RP + 2 * CELL_W));
        break;
    case O_LITERAL:
        PUSH(LOAD_CELL(EP));
        EP += CELL_W;
        break;
    case O_LITERALI:
        PUSH(A);
        NEXTC;
        break;
 throw:
    case O_THROW:
        /* EP can be unaligned in an error condition, so do BYTE * arithmetic */
        M0[2] = BAD = EP;
        if (!IN_MAIN_MEMORY((UCELL)*THROW) || !IS_ALIGNED((UCELL)*THROW))
            return -259;
        EP = (UCELL)*THROW;
        NEXTC;
        exception = 0; // Any exception has now been dealt with
        break;
    case O_HALT:
        return POP;
    case O_EPFETCH:
        PUSH(EP);
        break;
    case O_LIB:
        {
            CELL routine = POP;
            switch (routine) {
            case 0: /* ARGC ( -- u ) */
                PUSH(main_argc);
                break;

            case 1: /* ARG ( u1 -- c-addr u2 )*/
                {
                    UCELL narg = POP;
                    if (narg >= (UCELL)main_argc) {
                        PUSH(0);
                        PUSH(0);
                    } else {
                        PUSH(main_argv[narg]);
                        PUSH(main_argv_len[narg]);
                    }
                }
                break;

            case 2: /* STDIN */
                PUSH((CELL)(STDIN_FILENO));
                break;

            case 3: /* STDOUT */
                PUSH((CELL)(STDOUT_FILENO));
                break;

            case 4: /* STDERR */
                PUSH((CELL)(STDERR_FILENO));
                break;

            case 5: /* OPEN-FILE */
                {
                    bool binary = false;
                    int perm = getflags(POP, &binary);
                    UCELL len = POP;
                    UCELL str = POP;
                    char *file;
                    int res = getstr(str, len, &file);
                    int fd = res == 0 ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
                    free(file);
                    PUSH((CELL)fd);
                    PUSH(fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0);
                }
                break;

            case 6: /* CLOSE-FILE */
                {
                    int fd = POP;
                    PUSH((CELL)close(fd));
                }
                break;

            case 7: /* READ-FILE */
                {
                    int fd = POP;
                    UCELL nbytes = POP;
                    UCELL buf = POP;

                    ssize_t res = 0;
                    if (exception == 0) {
                        exception = beetle_pre_dma(buf, buf + nbytes);
                        if (exception == 0) {
                            res = read(fd, (BYTE *)M0 + buf, nbytes);
                            exception = beetle_post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH(res);
                    PUSH((exception == 0 && res >= 0) ? 0 : -1);
                }
                break;

            case 8: /* WRITE-FILE */
                {
                    int fd = POP;
                    UCELL nbytes = POP;
                    UCELL buf = POP;

                    ssize_t res = 0;
                    if (exception == 0) {
                        exception = beetle_pre_dma(buf, buf + nbytes);
                        if (exception == 0) {
                            res = write(fd, (BYTE *)M0 + buf, nbytes);
                            exception = beetle_post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH((exception == 0 && res >= 0) ? 0 : -1);
                }
                break;

            case 9: /* FILE-POSITION */
                {
                    int fd = POP;
                    off_t res = lseek(fd, 0, SEEK_CUR);
                    PUSH_DOUBLE((DUCELL)res);
                    PUSH(res >= 0 ? 0 : -1);
                }
                break;

            case 10: /* REPOSITION-FILE */
                {
                    int fd = POP;
                    DUCELL ud = POP_DOUBLE;
                    off_t res = lseek(fd, (off_t)ud, SEEK_SET);
                    PUSH(res >= 0 ? 0 : -1);
                }
                break;

            case 11: /* FLUSH-FILE */
                {
                    int fd = POP;
                    int res = fdatasync(fd);
                    PUSH(res);
                }
                break;

            case 12: /* RENAME-FILE */
                {
                    UCELL len1 = POP;
                    UCELL str1 = POP;
                    UCELL len2 = POP;
                    UCELL str2 = POP;
                    char *from;
                    int res = getstr(str2, len2, &from);
                    char *to;
                    if (res == 0)
                        res = getstr(str1, len1, &to);
                    if (res == 0)
                        res = rename(from, to);
                    free(from);
                    free(to);

                    PUSH(res);
                }
                break;

            case 13: /* DELETE-FILE */
                {
                    UCELL len = POP;
                    UCELL str = POP;
                    char *file;
                    int res = getstr(str, len, &file);
                    if (res == 0)
                        res = remove(file);
                    free(file);

                    PUSH(res);
                }
                break;

            default: /* Unimplemented LIB call */
                PUSH(-257);
                goto throw;
            }
        }
        break;
    case O_LINK:
        {
            CELL_pointer address;
            for (int i = POINTER_W - 1; i >= 0; i--)
                address.cells[i] = POP;
            address.pointer();
        }
        break;
    case O_RUN:
        break;
    case O_STEP:
        break;
    default:
        PUSH(-256);
        goto throw;
    }
    if (exception == 0)
        return -260; /* terminated OK */

    /* Deal with address exceptions during execution cycle. */
 badadr:
    SP -= CELL_W;
    if (!IN_MAIN_MEMORY(SP) || !IS_ALIGNED(SP))
      return -258;
    M0[SP / CELL_W] = exception; // FIXME
    goto throw;
}
