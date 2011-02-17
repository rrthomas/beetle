/* BEETLE.C

    (c) Reuben Thomas 1995-2011

    A user interface for Beetle.

*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>

#include "beetle.h"
#include "opcodes.h"
#include "debug.h"
#include "noecho.h"


/* User interface debug control */
bool debug = false;

#define MEMSIZE 16384	/* size of Beetle's memory in cells */

#define MAXLEN 80   /* maximum input line length */

static jmp_buf env;

static char *command[] = { ">D", ">R", "COUNTS", "DISASSEMBLE", "D>", "DATA",
    "DUMP", "FROM", "INITIALISE", "LOAD", "QUIT", "REGISTERS", "R>", "RETURN",
    "RUN", "STEP", "SAVE", "STACKS", "TRACE" };
enum commands { c_TOD, c_TOR, c_COUNTS, c_DISASSEMBLE, c_DFROM, c_DATA, c_DUMP,
    c_FROM, c_INITIALISE, c_LOAD, c_QUIT, c_REGISTERS, c_RFROM, c_RETURN, c_RUN,
    c_STEP, c_SAVE, c_STACKS, c_TRACE };
static int commands = 19;

static char *regist[] = { "A", "ADDRESS", "BAD", "CHECKED", "ENDISM", "EP", "I",
    "M0", "MEMORY", "RP", "R0", "SP", "S0", "THROW" };
enum registers { r_A, r_ADDRESS, r_BAD, r_CHECKED, r_ENDISM, r_EP, r_I,
    r_M0, r_MEMORY, r_RP, r_R0, r_SP, r_S0, r_THROW };
static int registers = 14;

static long count[256] = { 0 };


static int range(CELL adr, CELL limit, char *quantity)
{
    if (adr < limit) return 0;
    printf("%s must be in the range {0..%"PRIX32"h} ({0..%"PRId32"})\n", quantity, limit,
        limit);
    return 1;
}


static void upper(char *s)
{
    size_t i, len = strlen(s);

    for (i = 0; i < len; i++, s++)
        *s = toupper(*s);
}

static size_t search(char *token, char *list[], size_t entries)
{
    size_t i, len = strlen(token);

    for (i = 0; i < entries; i++)
        if (strncmp(token, list[i], len) == 0)
            return i;

    return SIZE_MAX;
}


static long single_arg(char *s)
{
    long n;
    char *endp;
    int len;

    if (s == NULL) { printf("Too few arguments\n"); longjmp(env, 1); }
    len = strlen(s);

    if (s[len - 1] == 'H' || s[len - 1] == 'h') {
        n = strtol(s, &endp, 16);
        len -= 1;
    }
    else n = strtol(s, &endp, 10);

    if (endp != &s[len]) { printf("Invalid number\n"); longjmp(env, 1); }

    return n;
}

static void double_arg(char *s, long *start, long *end)
{
    char *token, copy[MAXLEN];
    size_t i;
    bool plus = false;

    if (s == NULL) { printf("Too few arguments\n"); longjmp(env, 1); }

    strcpy(copy, s);
    if ((token = strtok(copy, " +")) == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }

    for (i = strlen(token); s[i] == ' ' && i < strlen(s); i++)
        ;
    if (i < strlen(s))
        plus = s[i] == '+';

    *start = single_arg(token);

    if ((token = strtok(NULL, " +")) == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }

    *end = single_arg(token);

    if (plus) *end += *start;
}


static int load_op(BYTE o)
{
    return (o == O_BRANCH || o == O_QBRANCH || o == O_CALL || o == O_LOOP ||
        o == O_PLOOP || o == O_LITERAL);
}

static int imm_op(BYTE o)
{
    return (load_op(o & 0xFE));
}

static void disassemble(CELL start, CELL end)
{
    CELL a, *p = (CELL *)(M0 + start);
    BYTE i;
    const char *token;

    while ((BYTE *)p < M0 + end) {
        printf("%08"PRIX32"h: ", (CELL)((BYTE *)p - M0));
        a = *p++;

        do {
            i = (BYTE)a;
            ARSHIFT(a, 8);
            token = disass(i);
            if (strcmp(token, ""))
                printf("%s", token);
            else
                printf("Undefined instruction");

            if (load_op(i)) {
                if (i != O_LITERAL)
                    printf(" %"PRIX32"h", *p);
                else
                    printf(" %"PRId32" (%"PRIX32"h)", *p, *p);
                p++;
            } else {
                if (imm_op(i)) {
                    if (i != O_LITERALI)
                        printf(" %"PRIX32"h", a * 4 + ((BYTE *)p - M0));
                    else
                        printf(" %"PRId32" (%"PRIX32"h)", a, a);
                    a = 0;
                }
            }

            printf("\n           ");
        } while (a != 0 && a != -1);
        putchar('\r');
    }
}


static void do_ass(char *token)
{
    int no = search(token, regist, registers);
    char *number = strtok(NULL, " ");
    int len = strlen(number);
    long value;
    int byte = 0;

    upper(number);
    if (number[0] == 'O') {
        value = toass(number + 1);
        if (value == 0xfe) {
            printf("Invalid opcode\n");
            return;
        }
        byte = 1;
    } else {
        value = single_arg(number);
        if (number[len - 1] == 'H' && len < 4)
            byte = 1;
        else {
            if ((unsigned long)value < 10)
                byte = len == 1;
            else if ((unsigned long)value < 100)
                byte = len == 2;
            else if ((unsigned long)value < 255)
                byte = len == 3;
        }
    }

    switch (no) {
        case r_A:
            if (debug)
                printf("Assign A %lX\n", value);
            A = value;
            break;
        case r_ADDRESS:
            if (debug)
                printf("Assign -ADDRESS %lX\n", value);
            *((CELL *)M0 + 3) = ADDRESS = value;
            break;
        case r_BAD:
            if (debug)
                printf("Assign 'BAD %lX\n", value);
            *((CELL *)M0 + 2) = BAD = value;
            break;
        case r_CHECKED:
            printf("Can't assign to CHECKED\n");
            break;
        case r_ENDISM:
            printf("Can't assign to ENDISM\n");
            break;
        case r_EP:
            if (range(value, MEMORY, "EP"))
                return;
            if (value & 3) {
                printf("EP must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign EP %lX\n", value);
            EP = (CELL *)(M0 + value);
            break;
        case r_I:
            if (value > 255) {
                printf("I must be assigned a byte\n");
                break;
            }
            if (debug)
                printf("Assign I %lX\n", value);
            I = value;
            break;
        case r_M0:
            printf("Can't assign to M0\n");
            break;
        case r_MEMORY:
            printf("Can't assign to MEMORY\n");
            break;
        case r_RP:
            if (range(value, MEMORY + 1, "RP"))
                return;
            if (value & 3) {
                printf("RP must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign RP %lX\n", value);
            RP = (CELL *)(M0 + value);
            break;
        case r_R0:
            if (range(value, MEMORY + 1, "R0"))
                return;
            if (value & 3) {
                printf("R0 must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign R0 %lX\n", value);
            R0 = (CELL *)(M0 + value);
            break;
        case r_SP:
            if (range(value, MEMORY + 1, "SP"))
                return;
            if (value & 3) {
                printf("SP must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign SP %lX\n", value);
            SP = (CELL *)(M0 + value);
            break;
        case r_S0:
            if (range(value, MEMORY + 1, "S0"))
                return;
            if (value & 3) {
                printf("S0 must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign S0 %lX\n", value);
            S0 = (CELL *)(M0 + value);
            break;
        case r_THROW:
            if (range(value, MEMORY, "'THROW"))
                return;
            if (value & 3) {
                printf("'THROW must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign 'THROW %lX\n", value);
            *THROW = value;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                if (range(adr, MEMORY, "Address"))
                    return;
                if ((adr & 3) && byte == 0) {
                    printf("Only a byte can be assigned to an unaligned "
                        "address\n");
                    return;
                }
                if (debug)
                    printf("Assign %lX to memory location %"PRIX32"%s\n", value, adr,
                           ((byte == 1 && (adr & 3) == 0) ? " (byte)" : ""));
                if (byte == 1)
                    *(M0 + FLIP(adr)) = (BYTE)value;
                else
                    *(CELL *)(M0 + adr) = value;
            }
    }
}

static void do_display(char *token, char *format)
{
    char display[80];
    int no = search(token, regist, registers);

    switch (no) {
        case r_A:
            sprintf(display, "A = %"PRIX32"h", A);
            break;
        case r_ADDRESS:
            sprintf(display, "-ADDRESS = %"PRIX32"h (%"PRId32")", ADDRESS, ADDRESS);
            break;
        case r_BAD:
            sprintf(display, "'BAD = %"PRIX32"h (%"PRId32")", BAD, BAD);
            break;
        case r_CHECKED:
            sprintf(display, "CHECKED = %d", CHECKED);
            break;
        case r_ENDISM:
            sprintf(display, "ENDISM = %d", ENDISM);
            break;
        case r_EP:
            sprintf(display, "EP = %05"PRIX32"h (%"PRId32")", (CELL)((BYTE *)EP - M0),
                    (CELL)((BYTE *)EP - M0));
            break;
        case r_I:
            sprintf(display, "I = %-10s (%02Xh)", disass(I), I);
            break;
        case r_M0:
            sprintf(display, "M0 = %p", M0);
            break;
        case r_MEMORY:
            sprintf(display, "MEMORY = %"PRIX32"h (%"PRId32")", MEMORY, MEMORY);
            break;
        case r_RP:
            sprintf(display, "RP = %"PRIX32"h (%"PRId32")", (CELL)((BYTE *)RP - M0),
                    (CELL)((BYTE *)RP - M0));
            break;
        case r_R0:
            sprintf(display, "R0 = %"PRIX32"h (%"PRId32")", (CELL)((BYTE *)R0 - M0),
                    (CELL)((BYTE *)R0 - M0));
            break;
        case r_SP:
            sprintf(display, "SP = %"PRIX32"h (%"PRId32")", (CELL)((BYTE *)SP - M0),
                    (CELL)((BYTE *)SP - M0));
            break;
        case r_S0:
            sprintf(display, "S0 = %"PRIX32"h (%"PRId32")", (CELL)((BYTE *)S0 - M0),
                    (CELL)((BYTE *)S0 - M0));
            break;
        case r_THROW:
            sprintf(display, "'THROW = %"PRIX32"h (%"PRId32")", *THROW, *THROW);
            break;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                if (debug)
                    printf("Display contents of memory location %"PRIX32"\n", adr);
                if (range(adr, MEMORY, "Address"))
                    return;
                if (adr & 3)
                    sprintf(display, "%"PRIX32"h: %Xh (%d) (byte)", adr,
                            *(M0 + FLIP(adr)), *(M0 + FLIP(adr)));
                else
                    sprintf(display, "%"PRIX32"h: %"PRIX32"h (%"PRId32") (cell)", adr,
                            *(CELL *)(M0 + adr), *(CELL *)(M0 + adr));
            }
    }
    printf(format, display);
}

static void do_registers(void)
{
    if (debug)
        printf("Display EP, I and A\n");
    do_display("EP", "%-25s");
    do_display("I", "%-22s");
    do_display("A", "%-16s\n");
}

static void do_command(int no)
{
    switch (no) {
        case c_TOD:
            {
                long value = single_arg(strtok(NULL, " "));

                if (debug)
                    printf("Push %ld on to the data stack\n", value);
                if (range((CELL)((BYTE *)SP - M0), MEMORY + 1, "SP"))
                    return;
                if ((BYTE *)SP - MEMORY == 0) {
                    printf("SP is 0h: no more stack items can be pushed\n");
                    return;
                }
                *--SP = value;
            }
            break;
        case c_TOR:
            {
                long value = single_arg(strtok(NULL, " "));

                if (debug)
                    printf("Push %ld on to the return stack\n", value);
                if (range((CELL)((BYTE *)SP - M0), MEMORY + 1, "RP"))
                    return;
                if ((BYTE *)RP - MEMORY == 0) {
                    printf("RP is 0h: no more stack items can be pushed\n");
                    return;
                }
                *--RP = value;
            }
            break;
        case c_COUNTS:
            {
                int i;

                for (i = 0; i < 92; i++) {
                    printf("%10s: %7ld", disass(i), count[i]);
                    if ((i + 1) % 4)
                        putchar(' ');
                    else
                        putchar('\n');
                }
                printf("%10s: %7ld\n", disass(255), count[255]);
            }
            break;
        case c_DISASSEMBLE:
            {
                long start, end;

                double_arg(strtok(NULL, ""), &start, &end);
                if (range(start, MEMORY, "Address"))
                    return;
                if (range(end, MEMORY, "Address"))
                    return;
                if (start >= end) {
                    printf("Start address must be less than end address\n");
                    return;
                }
                if (start & 3 || end & 3) {
                    printf("Address/offset must be cell-aligned\n");
                    return;
                }
                if (debug)
                    printf("Disassemble from %lX to %lX\n", start, end);
                disassemble((CELL)start, (CELL)end);
            }
            break;
        case c_DFROM:
            if (debug)
                printf("Pop a number from the data stack and display it\n");
            if (range((CELL)((BYTE *)SP - M0), MEMORY, "SP"))
                return;
            printf("%"PRId32" (%"PRIX32"h)\n", *SP, *SP);
            SP++;
            break;
        case c_DATA:
        case c_STACKS:
            if (debug)
                printf("Display the data stack\n");
            if (!range((CELL)((BYTE *)RP - M0), MEMORY + 1, "SP") &&
                !range((CELL)((BYTE *)S0 - M0), MEMORY + 1, "S0")) {
                if (SP == S0)
                    printf("Data stack empty\n");
                else if (SP > S0)
                    printf("Data stack underflow\n");
                else
                    show_data_stack();
            }
            if (no == c_STACKS) goto c_ret;
            break;
        case c_DUMP:
            {
                long start, end;
                int i;

                double_arg(strtok(NULL, ""), &start, &end);
                if (range(start, MEMORY, "Address")) return;
                if (range(end, MEMORY, "Address")) return;
                if (start >= end) {
                    printf("Start address must be less than end address\n");
                    return;
                }
                if (debug)
                    printf("Dump memory from %lX to %lX\n", start, end);
                while (start < end) {
                    printf("%08lXh: ", start);
                    for (i = 0; i < 8 && start < end; i++, start++)
                        printf("%02X ", M0[start]);
                    putchar('\n');
                }
            }
            break;
        case c_FROM:
            {
                char *arg = strtok(NULL, " ");
                long adr;

                if (arg != NULL) {
                    adr = single_arg(arg);
                    if (range(adr, MEMORY, "EP"))
                        return;
                    if (adr & 3) {
                        printf("Address must be cell-aligned\n");
                        break;
                    }
                    if (debug)
                        printf("Set EP to %lXh\n", adr);
                    EP = (CELL *)(M0 + adr);
                }
                if (debug)
                    printf("Perform NEXT\n");
                NEXT;
            }
            break;
        case c_INITIALISE:
        case c_LOAD:
            {
                long i;
                if (debug)
                    printf("Initialise Beetle\n");
                for (i = 0; i < MEMSIZE; i++) ((CELL*)M0)[i] = 0;
                for (i = 0; i < 256; i++) count[i] = 0;
                init_beetle(M0, MEMSIZE, 16);
                S0 = SP;
                R0 = RP;
                *THROW = 0;
                A = 0;
        if (no == c_LOAD) goto c_load;
        break;
            }
c_load:     {
                const char *file = strtok(NULL, " ");
                long adr = single_arg(strtok(NULL, " "));
                FILE *handle;
                int ret;

                if (adr & 3) {
                    printf("Address must be cell-aligned\n");
                    return;
                }
                if ((handle = fopen(file, "rb")) == NULL) {
                    printf("Cannot open file %s\n", file);
                    return;
                }
                if (debug)
                    printf("Load binary image %s to address %lX\n", file, adr);
                ret = load_object(handle, (CELL *)(M0 + adr));
                fclose(handle);

                switch (ret) {
                    case -1:
                        printf("Address out of range, or module too large\n");
                        break;
                    case -2:
                        printf("Module header invalid\n");
                        break;
                    case -3:
                        printf("Error while loading module\n");
                        break;
                }
            }
            break;
        case c_QUIT:
            restore_keyb();
            exit(0);
        case c_REGISTERS:
            do_registers();
            break;
        case c_RFROM:
            if (debug)
                printf("Pop a number from the return stack and display it\n");
            if (range((CELL)((BYTE *)RP - M0), MEMORY, "RP")) return;
            printf("%"PRIX32"h (%"PRId32")\n", *RP, *RP);
            RP++;
            break;
c_ret:      case c_RETURN:
            if (debug)
                printf("Display the return stack\n");
            if (range((CELL)((BYTE *)RP - M0), MEMORY + 1, "RP"))
                return;
            if (range((CELL)((BYTE *)R0 - M0), MEMORY + 1, "R0"))
                return;
            if (RP == R0) {
                printf("Return stack empty\n");
                return;
            }
            if (RP > R0) {
                printf("Return stack underflow\n");
                return;
            }
            show_return_stack();
            break;
        case c_RUN:
            {
                CELL ret;

                ret = run();
                printf("HALT code %"PRId32" was returned\n", ret);
            }
            break;
        case c_STEP:
        case c_TRACE:
            {
                char *arg = strtok(NULL, " ");
                unsigned long i, limit;
                CELL ret = 0;

                if (arg == NULL) {
                    if (debug)
                        printf("Step once\n");
                    if ((ret = single_step()))
                        printf("HALT code %"PRId32" was returned\n", ret);
                    if (no == c_TRACE) do_registers();
                    count[I]++;
                } else {
                    upper(arg);
                    if (strcmp(arg, "TO") == 0) {
                limit = single_arg(strtok(NULL, ""));
                if (limit & 3) {
                    printf("Address must be cell-aligned\n");
                    return;
                }
                if (range(limit, MEMORY, "Address"))
                    return;
                if (debug)
                    printf("STEP TO %lX\n", limit);
                while ((unsigned long)((BYTE *)EP - M0) != limit && ret == 0) {
                    ret = single_step();
                    if (no == c_TRACE) do_registers();
                    count[I]++;
                }
                if (ret != 0)
                    printf("HALT code %"PRId32" was returned at EP = %Xh ",
                           ret, (BYTE *)EP - M0);
                    } else {
                        limit = single_arg(arg);
                        if (debug)
                            printf("STEP for %lu instructions\n", limit);
                        for (i = 0; i < limit && ret == 0; i++) {
                            ret = single_step();
                            if (no == c_TRACE) do_registers();
                            count[I]++;
                        }
                        if (ret != 0)
                            printf("HALT code %"PRId32" was returned after %lu "
                                "steps\n", ret, i);
                    }
                }
            }
            break;
        case c_SAVE:
            {
                const char *file = strtok(NULL, " ");
                long start, end;
                FILE *handle;
                int ret;

                double_arg(strtok(NULL, ""), &start, &end);

                if (start & 3 || end & 3) {
                    printf("Address/offset must be cell-aligned\n");
                    return;
                }
                if (start >= end) {
                    printf("Start address must be less than end address\n");
                    return;
                }
                if ((handle = fopen(file, "wb")) == NULL) {
                    printf("Cannot open file %s\n", file);
                    return;
                }
                if (debug)
                    printf("Save memory to file %s from %lX to %lX\n", file, start,
                           end);
                ret = save_object(handle, (CELL *)(M0 + start),
                    (UCELL)((end - start) / CELL_W));
                fclose(handle);

                switch (ret) {
                    case -1:
                        printf("Address out of range or save area extends "
                            "beyond MEMORY\n");
                        break;
                    case -3:
                        printf("Error while saving module\n");
                        break;
                }
            }
            break;
    }
}


static void parse(char *input)
{
    char *token, copy[MAXLEN];
    size_t i, no;
    bool ass = false;

    if (input[0] == '!') {
        if (debug)
            printf("Send %s to the environment\n", input + 1);
        system(input + 1);
        return;
    }

    strcpy(copy, input);
    token = strtok(copy, strchr(copy, '=') == NULL ? " " : "=");
    if (token == NULL || strlen(token) == 0) return;
    upper(token);

    for (i = strlen(token); input[i] == ' ' && i < strlen(input); i++)
        ;
    if (i < strlen(input))
        ass = input[i] == '=';

    no = search(token, command, commands);
    if (no != SIZE_MAX)
        do_command(no);
    else {
        if (ass) do_ass(token);
        else do_display(token, "%s\n");
    }
}


int main(int argc, char *argv[])
{
    char input[MAXLEN], *nl;
    long i;
    CELL *mem;

    if (argc > 2) {
        printf("Usage: beetle [OBJECT]\n");
        exit(1);
    }

    if (!(mem = (CELL *)calloc(CELL_W, MEMSIZE))) {
        printf("beetle: couldn't claim memory for Beetle\n");
        exit(1);
    }
    for (i = 0; i < 256; i++) count[i] = 0;
    init_beetle((BYTE *)mem, MEMSIZE, 16);
    S0 = SP;
    R0 = RP;
    *THROW = 0;
    A = 0;

    if (argc == 2) {
        FILE *handle;
        int ret;

        handle = fopen(argv[1], "r");
        load_object(handle, (CELL *)(M0 + 16));
        fclose(handle);

        init_keyb();
        ret = run();
        restore_keyb();

        return ret;
    }

    /* FIXME: Add some sort of version. */
    printf("Beetle shell\n(c) Reuben Thomas 1995-2011\n\n");

    while (1) {
        if (setjmp(env) == 0) {
            printf(">");
            fgets(input, MAXLEN, stdin);
            if ((nl = strrchr(input, '\n')))
                *nl = '\0';
            init_keyb();
            parse(input);
            restore_keyb();
        }
        else restore_keyb();
    }
}
