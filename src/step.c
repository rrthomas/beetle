// The interface calls run() : integer and single_step() : integer.
//
// (c) Reuben Thomas 1994-2021
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "external_syms.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "binary-io.h"
#include "minmax.h"
#include "verify.h"

#include "beetle.h"
#include "beetle_aux.h"
#include "private.h"
#include "beetle_opcodes.h"

#ifdef HAVE_MIJIT
#include "mijit-beetle/c-decls.h"

mijit_beetle_jit *beetle_jit;
#endif

#define BEETLE_DATA_CELLS 64
#define BEETLE_RETURN_CELLS 64


// VM registers

beetle_Registers beetle_registers;
beetle_CELL *M0;

// General memory access

// Return native address of a range of VM memory, or NULL if invalid
inline _GL_ATTRIBUTE_PURE uint8_t *native_address_of_range(UCELL start, UCELL length)
{
    if (start > R(MEMORY) || R(MEMORY) - start < length)
        return NULL;

    return ((uint8_t *)M0) + start;
}

// Macro for byte addressing
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (CELL_W - 1))
#else
#define FLIP(addr) (addr)
#endif

inline int load_cell(UCELL addr, CELL *value)
{
    if (!IS_ALIGNED(addr)) {
        R(NOT_ADDRESS) = addr;
        return -23;
    }

    uint8_t *ptr = native_address_of_range(addr, CELL_W);
    if (ptr == NULL) {
        R(NOT_ADDRESS) = addr;
        return -9;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *value = *(CELL *)ptr;
#pragma GCC diagnostic pop
    return 0;
}

inline int load_byte(UCELL addr, BYTE *value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL) {
        R(NOT_ADDRESS) = addr;
        return -9;
    }
    *value = *ptr;
    return 0;
}

inline int store_cell(UCELL addr, CELL value)
{
    if (!IS_ALIGNED(addr)) {
        R(NOT_ADDRESS) = addr;
        return -23;
    }

    uint8_t *ptr = native_address_of_range(addr, CELL_W);
    if (ptr == NULL) {
        R(NOT_ADDRESS) = addr;
        return -9;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(CELL *)ptr = value;
#pragma GCC diagnostic pop
    return 0;
}

inline int store_byte(UCELL addr, BYTE value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL) {
        R(NOT_ADDRESS) = addr;
        return -9;
    }
    *ptr = value;
    return 0;
}

_GL_ATTRIBUTE_CONST CELL reverse_cell(CELL value)
{
    CELL res = 0;
    for (unsigned i = 0; i < CELL_W / 2; i++)
    {
        unsigned lopos = CHAR_BIT * i;
        unsigned hipos = CHAR_BIT * (CELL_W - 1 - i);
        unsigned move = hipos - lopos;
        res |= ((((UCELL)value) & ((UCELL)CHAR_MASK << hipos)) >> move) | ((((UCELL)value) & ((UCELL)CHAR_MASK << lopos)) << move);
    }
    return res;
}

int reverse(UCELL start, UCELL length)
{
    int ret = 0;
    for (UCELL i = 0; ret == 0 && i < length; i++)
    {
        CELL c;
        ret = load_cell(start + i * CELL_W, &c) || store_cell(start + i, reverse_cell(c));
    }
    return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
int pre_dma(UCELL from, UCELL to)
{
    int exception = 0;

    // Expand range to words
    from &= -CELL_W;
    to = ALIGN(to);

    if (to < from || native_address_of_range(from, to - from) == NULL)
        exception = -1;
    if (exception == 0 && ENDISM)
        exception = reverse(from, to - from);
    return exception;
}
#pragma GCC diagnostic pop

int post_dma(UCELL from, UCELL to)
{
    return pre_dma(from, to);
}

// Initialise registers that are not fixed

int init(size_t size)
{
#ifdef HAVE_MIJIT
    beetle_jit = mijit_beetle_new();
    if (beetle_jit == NULL)
        return -1;
    R(CHECKED) = 0; // address checking is disabled in this implementation
#else
    R(CHECKED) = 1; // address checking is mandatory in this implementation
#endif

    R(MEMORY) = size * CELL_W;
    M0 = (beetle_CELL *)calloc(size, CELL_W);
    if (M0 == NULL)
        return -1;

    R(EP) = 0;
    R(A) = 0;
    R(S0) = R(SP) = R(MEMORY) - BEETLE_RETURN_CELLS * 4;
    R(R0) = R(RP) = R(MEMORY);
    R(THROW) = 0;
    R(BAD) = CELL_MAX;
    R(NOT_ADDRESS) = CELL_MAX;

    return 0;
}

void destroy(void)
{
    free(M0);
#ifdef HAVE_MIJIT
    mijit_beetle_drop(beetle_jit);
#endif
}


// Assumption for file functions
verify(sizeof(int) <= sizeof(CELL));


// Check whether a VM address points to a native cell-aligned cell
#define IS_VALID(a)                                     \
    (native_address_of_range((a), CELL_W) != NULL)

#define CHECK_VALID_CELL(a)                     \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, badadr)        \
    CHECK_ADDRESS(a, IS_VALID(a), -9, badadr)


// Division macros
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SGN(x) ((x) > 0 ? 1 : -1)  // not a proper sign function!

#define DIV_WOULD_OVERFLOW(a, b) (((a) == CELL_MIN) && ((b) == -1))
#define DIV_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? CELL_MIN : (a) / (b))
#define MOD_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? 0 : (a) % (b))

#define FDIV(a, b) (DIV_WITH_OVERFLOW((a), (b)) - ((((a) ^ (b)) < 0) && MOD_WITH_OVERFLOW((a), (b)) != 0))
#define FMOD(a, b, t) ((t = MOD_WITH_OVERFLOW((a), (b)), (((a) ^ (b)) >= 0 || t == 0)) ? t : \
                       SGN(b) * (ABS(b) - ABS(t)))

#define DIVZERO(x)                              \
    if (x == 0) {                               \
        PUSH(-10);                              \
        goto throw;                             \
    }


// I/O support

// Copy a string from VM to native memory
static int getstr(UCELL adr, UCELL len, char **res)
{
    int exception = 0;

    *res = calloc(1, len + 1);
    if (*res == NULL)
        exception = -511;
    else
        for (size_t i = 0; exception == 0 && i < len; i++, adr++) {
            exception = load_byte(adr, (BYTE *)((*res) + i));
        }

    return exception;
}

// Convert portable open(2) flags bits to system flags
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
        flags |= O_CREAT | O_TRUNC;

    if (perm & 8)
        *binary = true;

    return flags;
}

// Register command-line args
static int main_argc = 0;
static const char **main_argv;
static UCELL *main_argv_len;
int register_args(int argc, const char *argv[])
{
    main_argc = argc;
    main_argv = argv;
    if ((main_argv_len = calloc(argc, sizeof(UCELL))) == NULL)
        return -1;

    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (len > CELL_MAX)
            return -2;
        main_argv_len[i] = len;
    }
    return 0;
}

// Inner execution function
static CELL run_or_step(bool run)
{
    int exception = 0;
    do {
        CELL temp = 0, temp2 = 0;
        BYTE byte = 0;

        R(I) = (BYTE)R(A);
        ARSHIFT(R(A), 8);
        switch (R(I)) {
        case O_NEXT00:
        case O_NEXTFF:
        next:
            R(EP) += CELL_W;
            exception = load_cell(R(EP) - CELL_W, &R(A));
            break;
        case O_DUP:
            {
                CELL top = LOAD_CELL(R(SP));
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
                CELL next = LOAD_CELL(R(SP) - CELL_W * STACK_DIRECTION);
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
                UCELL depth = POP;
                CELL pickee = LOAD_CELL(R(SP) - depth * CELL_W * STACK_DIRECTION);
                PUSH(pickee);
            }
            break;
        case O_ROLL:
            {
                UCELL depth = POP;
                CELL rollee = LOAD_CELL(R(SP) - depth * CELL_W * STACK_DIRECTION);
                for (CELL i = depth; i > 0; i--)
                    STORE_CELL(R(SP) - i * CELL_W * STACK_DIRECTION,
                               LOAD_CELL(R(SP) - (i - 1) * CELL_W * STACK_DIRECTION));
                STORE_CELL(R(SP), rollee);
            }
            break;
        case O_QDUP:
            {
                CELL value = LOAD_CELL(R(SP));
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
                CELL value = LOAD_CELL(R(RP));
                PUSH(value);
            }
            break;
        case O_LESS:
            {
                CELL a = POP;
                CELL b = POP;
                PUSH(b < a ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_GREATER:
            {
                CELL a = POP;
                CELL b = POP;
                PUSH(b > a ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_EQUAL:
            {
                CELL a = POP;
                CELL b = POP;
                PUSH(a == b ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_NEQUAL:
            {
                CELL a = POP;
                CELL b = POP;
                PUSH(a != b ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_LESS0:
            {
                CELL a = POP;
                PUSH(a < 0 ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_GREATER0:
            {
                CELL a = POP;
                PUSH(a > 0 ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_EQUAL0:
            {
                CELL a = POP;
                PUSH(a == 0 ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_NEQUAL0:
            {
                CELL a = POP;
                PUSH(a != 0 ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_ULESS:
            {
                UCELL a = POP;
                UCELL b = POP;
                PUSH(b < a ? BEETLE_TRUE : BEETLE_FALSE);
            }
            break;
        case O_UGREATER:
            {
                UCELL a = POP;
                UCELL b = POP;
                PUSH(b > a ? BEETLE_TRUE : BEETLE_FALSE);
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
                UCELL a = POP;
                UCELL b = POP;
                PUSH(b + a);
            }
            break;
        case O_MINUS:
            {
                UCELL a = POP;
                UCELL b = POP;
                PUSH(b - a);
            }
            break;
        case O_SWAPMINUS:
            {
                UCELL a = POP;
                UCELL b = POP;
                PUSH(a - b);
            }
            break;
        case O_PLUS1:
            {
                UCELL a = POP;
                PUSH(a + 1);
            }
            break;
        case O_MINUS1:
            {
                UCELL a = POP;
                PUSH(a - 1);
            }
            break;
        case O_PLUSCELL:
            {
                UCELL a = POP;
                PUSH(a + CELL_W);
            }
            break;
        case O_MINUSCELL:
            {
                UCELL a = POP;
                PUSH(a - CELL_W);
            }
            break;
        case O_STAR:
            {
                UCELL multiplier = POP;
                UCELL multiplicand = POP;
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
                PUSH(MOD_WITH_OVERFLOW(dividend, divisor));
                PUSH(DIV_WITH_OVERFLOW(dividend, divisor));
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
                PUSH(beetle_LSHIFT(value, shift));
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
                PUSH(beetle_LSHIFT(value, 1));
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
                CELL value = R(SP);
                PUSH(value);
            }
            break;
        case O_SPSTORE:
            {
                CELL value = POP;
                CHECK_ALIGNED(value);
                R(SP) = value;
            }
            break;
        case O_RPFETCH:
            PUSH(R(RP));
            break;
        case O_RPSTORE:
            {
                CELL value = POP;
                CHECK_ALIGNED(value);
                R(RP) = value;
            }
            break;
        case O_BRANCH:
            {
                CELL addr = LOAD_CELL(R(EP));
                CHECK_VALID_CELL(addr);
                R(EP) = addr;
                goto next;
            }
            break;
        case O_BRANCHI:
            R(EP) += R(A) * CELL_W;
            goto next;
            break;
        case O_QBRANCH:
            if (POP == BEETLE_FALSE) {
                CELL addr = LOAD_CELL(R(EP));
                CHECK_VALID_CELL(addr);
                R(EP) = addr;
                goto next;
            } else
                R(EP) += CELL_W;
            break;
        case O_QBRANCHI:
            if (POP == BEETLE_FALSE)
                R(EP) += R(A) * CELL_W;
            goto next;
            break;
        case O_EXECUTE:
            {
                CELL addr = POP;
                CHECK_VALID_CELL(addr);
                PUSH_RETURN(R(EP));
                R(EP) = addr;
                goto next;
            }
            break;
        case O_FEXECUTE:
            {
                CELL addr = POP;
                CHECK_VALID_CELL(addr);
                PUSH_RETURN(R(EP));
                addr = LOAD_CELL(addr);
                CHECK_VALID_CELL(addr);
                R(EP) = addr;
                goto next;
            }
            break;
        case O_CALL:
            {
                PUSH_RETURN(R(EP) + CELL_W);
                CELL addr = LOAD_CELL(R(EP));
                CHECK_VALID_CELL(addr);
                R(EP) = addr;
                goto next;
            }
            break;
        case O_CALLI:
            PUSH_RETURN(R(EP));
            R(EP) += R(A) * CELL_W;
            goto next;
            break;
        case O_EXIT:
            {
                CELL addr = POP_RETURN;
                CHECK_VALID_CELL(addr);
                R(EP) = addr;
                goto next;
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
                CELL limit = LOAD_CELL(R(RP));
                PUSH_RETURN(index + 1);
                if (index + 1 == limit) {
                    (void)POP_RETURN;
                    (void)POP_RETURN;
                    R(EP) += CELL_W;
                } else {
                    CELL addr = LOAD_CELL(R(EP));
                    CHECK_VALID_CELL(addr);
                    R(EP) = addr;
                    goto next;
                }
            }
            break;
        case O_LOOPI:
            {
                CELL index = POP_RETURN;
                CELL limit = LOAD_CELL(R(RP));
                PUSH_RETURN(index + 1);
                if (index + 1 == limit) {
                    (void)POP_RETURN;
                    (void)POP_RETURN;
                } else
                    R(EP) += R(A) * CELL_W;
                goto next;
            }
            break;
        case O_PLOOP:
            {
                CELL index = POP_RETURN;
                CELL limit = LOAD_CELL(R(RP));
                CELL diff = index - limit;
                CELL inc = POP;
                PUSH_RETURN(index + inc);
                if ((((diff + inc) ^ diff) & (diff ^ inc)) < 0) {
                    (void)POP_RETURN;
                    (void)POP_RETURN;
                    R(EP) += CELL_W;
                } else {
                    CELL addr = LOAD_CELL(R(EP));
                    CHECK_VALID_CELL(addr);
                    R(EP) = addr;
                    goto next;
                }
            }
            break;
        case O_PLOOPI:
            {
                CELL index = POP_RETURN;
                CELL limit = LOAD_CELL(R(RP));
                CELL diff = index - limit;
                CELL inc = POP;
                PUSH_RETURN(index + inc);
                if ((((diff + inc) ^ diff) & (diff ^ inc)) < 0) {
                    (void)POP_RETURN;
                    (void)POP_RETURN;
                } else
                    R(EP) += R(A) * CELL_W;
                goto next;
            }
            break;
        case O_UNLOOP:
            (void)POP_RETURN;
            (void)POP_RETURN;
            break;
        case O_J:
            PUSH(LOAD_CELL(R(RP) - 2 * CELL_W * STACK_DIRECTION));
            break;
        case O_LITERAL:
            PUSH(LOAD_CELL(R(EP)));
            R(EP) += CELL_W;
            break;
        case O_LITERALI:
            PUSH(R(A));
            goto next;
            break;
        throw:
        case O_THROW:
            // exception may already be set, so CELL_STORE may have no effect here.
            R(BAD) = R(EP);
            if (!IS_VALID(R(THROW)) || !IS_ALIGNED(R(THROW)))
                return EXIT_INVALID_THROW;
            R(EP) = R(THROW);
            exception = 0; // Any exception has now been dealt with
            goto next;
            break;
        case O_HALT:
            return POP;
        case O_EPFETCH:
            PUSH(R(EP));
            break;
        case O_S0FETCH:
            PUSH(R(S0));
            break;
        case O_S0STORE:
            {
                CELL value = POP;
                CHECK_ALIGNED(value);
                R(S0) = value;
            }
            break;
        case O_R0FETCH:
            PUSH(R(R0));
            break;
        case O_R0STORE:
            {
                CELL value = POP;
                CHECK_ALIGNED(value);
                R(R0) = value;
            }
            break;
        case O_THROWFETCH:
            PUSH(R(THROW));
            break;
        case O_THROWSTORE:
            {
                CELL value = POP;
                CHECK_ALIGNED(value);
                R(THROW) = value;
            }
            break;
        case O_MEMORYFETCH:
            PUSH(R(MEMORY));
            break;
        case O_BADFETCH:
            PUSH(R(BAD));
            break;
        case O_NOT_ADDRESSFETCH:
            PUSH(R(NOT_ADDRESS));
            break;
        case O_LIB:
            {
                CELL routine = POP;
                switch (routine) {
                case 0: // BL
                    PUSH((CELL)' ');
                    break;
                case 1: // CR
                    {
                        char c = '\n';
                        write(STDOUT_FILENO, &c, 1);
                    }
                    break;
                case 2: // EMIT
                    {
                        char c = (char)POP;
                        write(STDOUT_FILENO, &c, 1);
                    }
                    break;
                case 3: // KEY
                    {
                        char c;
                        read(STDIN_FILENO, &c, 1);
                        PUSH((CELL)c);
                    }
                    break;
                case 4: // OPEN-FILE
                    {
                        bool binary = false;
                        int perm = getflags(POP, &binary);
                        UCELL len = POP;
                        UCELL str = POP;
                        char *file;
                        exception = getstr(str, len, &file);
                        int fd = exception == 0 ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
                        free(file);
                        PUSH((CELL)fd);
                        PUSH(fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0);
                    }
                    break;
                case 5: // CLOSE-FILE
                    {
                        int fd = POP;
                        PUSH((CELL)close(fd));
                    }
                    break;
                case 6: // READ-FILE
                    {
                        int fd = POP;
                        UCELL nbytes = POP;
                        UCELL buf = POP;

                        ssize_t res = 0;
                        if (exception == 0) {
                            exception = pre_dma(buf, buf + nbytes);
                            if (exception == 0) {
                                res = read(fd, native_address_of_range(buf, 0), nbytes);
                                exception = post_dma(buf, buf + nbytes);
                            }
                        }

                        PUSH(res);
                        PUSH((exception == 0 && res >= 0) ? 0 : -1);
                    }
                    break;
                case 7: // WRITE-FILE
                    {
                        int fd = POP;
                        UCELL nbytes = POP;
                        UCELL buf = POP;

                        ssize_t res = 0;
                        if (exception == 0) {
                            exception = pre_dma(buf, buf + nbytes);
                            if (exception == 0) {
                                res = write(fd, native_address_of_range(buf, 0), nbytes);
                                exception = post_dma(buf, buf + nbytes);
                            }
                        }

                        PUSH((exception == 0 && res >= 0) ? 0 : -1);
                    }
                    break;
                case 8: // FILE-POSITION
                    {
                        int fd = POP;
                        off_t res = lseek(fd, 0, SEEK_CUR);
                        PUSH_DOUBLE((DUCELL)res);
                        PUSH(res >= 0 ? 0 : -1);
                    }
                    break;
                case 9: // REPOSITION-FILE
                    {
                        int fd = POP;
                        DUCELL ud = POP_DOUBLE;
                        off_t res = lseek(fd, (off_t)ud, SEEK_SET);
                        PUSH(res >= 0 ? 0 : -1);
                    }
                    break;
                case 10: // FLUSH-FILE
                    {
                        int fd = POP;
                        int res = fdatasync(fd);
                        PUSH(res);
                    }
                    break;
                case 11: // RENAME-FILE
                    {
                        UCELL len1 = POP;
                        UCELL str1 = POP;
                        UCELL len2 = POP;
                        UCELL str2 = POP;
                        char *from;
                        char *to = NULL;
                        exception = getstr(str2, len2, &from) ||
                            getstr(str1, len1, &to) ||
                            rename(from, to);
                        free(from);
                        free(to);
                        PUSH(exception);
                    }
                    break;
                case 12: // DELETE-FILE
                    {
                        UCELL len = POP;
                        UCELL str = POP;
                        char *file;
                        exception = getstr(str, len, &file) ||
                            remove(file);
                        free(file);
                        PUSH(exception);
                    }
                    break;
                case 13: // FILE-SIZE
                    {
                        struct stat st;
                        int fd = POP;
                        int res = fstat(fd, &st);
                        PUSH_DOUBLE((DUCELL)st.st_size);
                        PUSH(res);
                    }
                    break;
                case 14: // RESIZE-FILE
                    {
                        int fd = POP;
                        DUCELL ud = POP_DOUBLE;
                        int res = ftruncate(fd, (off_t)ud);
                        PUSH(res);
                    }
                    break;
                case 15: // FILE-STATUS
                    {
                        struct stat st;
                        int fd = POP;
                        int res = fstat(fd, &st);
                        PUSH(st.st_mode);
                        PUSH(res);
                    }
                    break;
                case 16: // ARGC ( -- u )
                    PUSH(main_argc);
                    break;
                case 17: // ARGLEN ( u1 -- u2 )
                    {
                        UCELL narg = POP;
                        if (narg >= (UCELL)main_argc)
                            PUSH(0);
                        else
                            PUSH(main_argv_len[narg]);
                    }
                    break;
                case 18: // ARGCOPY ( u1 addr -- )
                    {
                        UCELL addr = POP;
                        UCELL narg = POP;
                        if (narg < (UCELL)main_argc) {
                            UCELL len = (UCELL)main_argv_len[narg];
                            char *ptr = (char *)native_address_of_range(addr, len);
                            if (ptr != NULL) {
                                UCELL end = ALIGN(addr + len);
                                pre_dma(addr, end);
                                strncpy(ptr, main_argv[narg], len);
                                post_dma(addr, end);
                            }
                        }
                    }
                    break;
                case 19: // STDIN
                    PUSH((CELL)(STDIN_FILENO));
                    break;
                case 20: // STDOUT
                    PUSH((CELL)(STDOUT_FILENO));
                    break;
                case 21: // STDERR
                    PUSH((CELL)(STDERR_FILENO));
                    break;
                default: /* Unimplemented LIB call */
                    exception = EXIT_UNIMPLEMENTED_LIB;
                    break;
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

        default:
            exception = EXIT_INVALID_OPCODE;
            break;
        }

        if (exception != 0) {
            // Deal with address exceptions during execution cycle.
            // We must push the exception code "manually", as PUSH does nothing
            // when exception is non-zero. Also, we must return a different
            // code from usual if SP is now invalid.
        badadr:
            R(SP) += CELL_W * STACK_DIRECTION;
            if (!IS_VALID(R(SP)) || !IS_ALIGNED(R(SP)))
                return EXIT_INVALID_SP;
            store_cell(R(SP), exception);
            goto throw;
        }
    } while (run == true);

    if (exception == 0 && run == false)
        exception = EXIT_SINGLE_STEP;
    return exception;
}

// Perform one pass of the execution cycle
CELL single_step(void)
{
    return run_or_step(false);
}

CELL run(void)
{
#ifdef HAVE_MIJIT
    CELL exception;
    do {
        mijit_beetle_run(
            beetle_jit,
            (uint32_t *)M0,
            (mijit_beetle_registers *)(void *)&beetle_registers
        );
        exception = single_step();
    } while (exception == EXIT_SINGLE_STEP);
    return exception;
#else
    return run_or_step(true);
#endif
}
