/* BEETLE.C

    (c) Reuben Thomas 1995-2018

    A user interface for Beetle.

*/


#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <getopt.h>
#include <sys/wait.h>

#include "progname.h"
#include "xvasprintf.h"

#include "beetle.h"
#include "opcodes.h"
#include "debug.h"


bool debug = false; // User interface debug control

#define DEFAULT_MEMORY 1048576 // Default size of Beetle's memory in cells (4Mb)
#define MAX_MEMORY 536870912 // Maximum size of memory in cells (2Gb)
static UCELL memory_size = DEFAULT_MEMORY; // Size of Beetle's memory in cells

#define MAXLEN 80   // Maximum input line length

static jmp_buf env;

static const char *command[] = {
    ">D", ">R", "COUNTS", "DISASSEMBLE", "D>", "DATA",
    "DEBUG", "DUMP", "FROM", "INITIALISE", "LOAD", "QUIT", "REGISTERS", "R>",
    "RETURN", "RUN", "STEP", "SAVE", "STACKS", "TRACE"
};
enum commands { c_TOD, c_TOR, c_COUNTS, c_DISASSEMBLE, c_DFROM, c_DATA,
    c_DEBUG, c_DUMP, c_FROM, c_INITIALISE, c_LOAD, c_QUIT, c_REGISTERS, c_RFROM,
    c_RETURN, c_RUN, c_STEP, c_SAVE, c_STACKS, c_TRACE };
static int commands = sizeof(command) / sizeof(*command);

static const char *regist[] = {
    "A", "-ADDRESS", "'BAD", "CHECKED", "ENDISM", "EP", "I",
    "M0", "MEMORY", "RP", "R0", "SP", "S0", "'THROW"
};
enum registers { r_A, r_NOT_ADDRESS, r_BAD, r_CHECKED, r_ENDISM, r_EP, r_I,
    r_M0, r_MEMORY, r_RP, r_R0, r_SP, r_S0, r_THROW };
static int registers = sizeof(regist) / sizeof(*regist);

static long count[256];


static int range(CELL adr, CELL limit, const char *quantity)
{
    if (adr < limit) return 0;
    printf("%s must be in the range {0..%"PRIX32"h} ({0..%"PRIu32"})\n",
           quantity, (UCELL)limit, (UCELL)limit);
    return 1;
}


static void upper(char *s)
{
    size_t i, len = strlen(s);

    for (i = 0; i < len; i++, s++)
        *s = toupper(*s);
}

static size_t search(const char *token, const char *list[], size_t entries)
{
    size_t i, len = strlen(token);

    for (i = 0; i < entries; i++)
        if (strncmp(token, list[i], len) == 0)
            return i;

    return SIZE_MAX;
}


static long single_arg(const char *s)
{
    long n;
    char *endp;
    int len;

    if (s == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }
    len = strlen(s);

    if (s[len - 1] == 'H' || s[len - 1] == 'h') {
        n = strtol(s, &endp, 16);
        len -= 1;
    }
    else n = strtol(s, &endp, 10);

    if (endp != &s[len]) {
        printf("Invalid number\n");
        longjmp(env, 1);
    }

    return n;
}

static void double_arg(char *s, long *start, long *end)
{
    char *token, copy[MAXLEN];
    size_t i;
    bool plus = false;

    if (s == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }

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

    if (plus)
        *end += *start;
}


static int load_op(BYTE o)
{
    return o == O_BRANCH || o == O_QBRANCH || o == O_CALL || o == O_LOOP ||
        o == O_PLOOP || o == O_LITERAL;
}

static int imm_op(BYTE o)
{
    return load_op(o & 0xFE);
}

static void disassemble(CELL start, CELL end)
{
    CELL a, *p = M0 + start;
    BYTE i;
    const char *token;

    while (p < M0 + end) {
        printf("%08"PRIX32"h: ", (UCELL)((p - M0) * CELL_W));
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
                    printf(" %"PRIX32"h", (UCELL)*p);
                else
                    printf(" %"PRId32" (%"PRIX32"h)", *p, (UCELL)*p);
                p++;
            } else {
                if (imm_op(i)) {
                    if (i != O_LITERALI)
                        printf(" %"PRIX32"h", (UCELL)((p - M0 + a) * CELL_W));
                    else
                        printf(" %"PRId32" (%"PRIX32"h)", a, (UCELL)a);
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
                printf("Assign A %lX\n", (unsigned long)value);
            A = value;
            break;
        case r_NOT_ADDRESS:
            if (debug)
                printf("Assign -ADDRESS %lX\n", (unsigned long)value);
            M0[3] = NOT_ADDRESS = value;
            break;
        case r_BAD:
            if (debug)
                printf("Assign 'BAD %lX\n", (unsigned long)value);
            M0[2] = BAD = value;
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
                printf("Assign EP %lX\n", (unsigned long)value);
            EP = value;
            break;
        case r_I:
            if (value > 255) {
                printf("I must be assigned a byte\n");
                break;
            }
            if (debug)
                printf("Assign I %lX\n", (unsigned long)value);
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
                printf("Assign RP %lX\n", (unsigned long)value);
            RP = value;
            break;
        case r_R0:
            if (range(value, MEMORY + 1, "R0"))
                return;
            if (value & 3) {
                printf("R0 must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign R0 %lX\n", (unsigned long)value);
            R0 = value;
            break;
        case r_SP:
            if (range(value, MEMORY + 1, "SP"))
                return;
            if (value & 3) {
                printf("SP must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign SP %lX\n", (unsigned long)value);
            SP = value;
            break;
        case r_S0:
            if (range(value, MEMORY + 1, "S0"))
                return;
            if (value & 3) {
                printf("S0 must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign S0 %lX\n", (unsigned long)value);
            S0 = value;
            break;
        case r_THROW:
            if (range(value, MEMORY, "'THROW"))
                return;
            if (value & 3) {
                printf("'THROW must be cell-aligned\n");
                break;
            }
            if (debug)
                printf("Assign 'THROW %lX\n", (unsigned long)value);
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
                    printf("Assign %lX to memory location %"PRIX32"%s\n",
                           (unsigned long)value, (UCELL)adr,
                           ((byte == 1 && (adr & 3) == 0) ? " (byte)" : ""));
                if (byte == 1)
                    beetle_store_byte(adr, value);
                else
                    beetle_store_cell(adr, value);
            }
    }
}

static void do_display(const char *token, const char *format)
{
    char *display;
    int no = search(token, regist, registers);

    switch (no) {
        case r_A:
            display = xasprintf("A = %"PRIX32"h", (UCELL)A);
            break;
        case r_NOT_ADDRESS:
            display = xasprintf("-ADDRESS = %"PRIX32"h (%"PRIu32")", NOT_ADDRESS, NOT_ADDRESS);
            break;
        case r_BAD:
            display = xasprintf("'BAD = %"PRIX32"h (%"PRIu32")", BAD, BAD);
            break;
        case r_CHECKED:
            display = xasprintf("CHECKED = %d", CHECKED);
            break;
        case r_ENDISM:
            display = xasprintf("ENDISM = %d", ENDISM);
            break;
        case r_EP:
            display = xasprintf("EP = %05"PRIX32"h (%"PRIu32")", EP, EP);
            break;
        case r_I:
            display = xasprintf("I = %-10s (%02Xh)", disass(I), I);
            break;
        case r_M0:
            display = xasprintf("M0 = %p", M0);
            break;
        case r_MEMORY:
            display = xasprintf("MEMORY = %"PRIX32"h (%"PRIu32")", MEMORY, MEMORY);
            break;
        case r_RP:
            display = xasprintf("RP = %"PRIX32"h (%"PRIu32")", RP, RP);
            break;
        case r_R0:
            display = xasprintf("R0 = %"PRIX32"h (%"PRIu32")", R0, R0);
            break;
        case r_SP:
            display = xasprintf("SP = %"PRIX32"h (%"PRIu32")", SP, SP);
            break;
        case r_S0:
            display = xasprintf("S0 = %"PRIX32"h (%"PRIu32")", S0, S0);
            break;
        case r_THROW:
            display = xasprintf("'THROW = %"PRIX32"h (%"PRIu32")", (UCELL)*THROW, (UCELL)*THROW);
            break;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                if (debug)
                    printf("Display contents of memory location %"PRIX32"\n", (UCELL)adr);
                if (range(adr, MEMORY, "Address"))
                    return;
                if (!IS_ALIGNED(adr)) {
                    BYTE b;
                    beetle_load_byte(adr, &b);
                    display = xasprintf("%"PRIX32"h: %Xh (%d) (byte)", (UCELL)adr,
                                        b, b);
                } else {
                    CELL c;
                    beetle_load_cell(adr, &c);
                    display = xasprintf("%"PRIX32"h: %"PRIX32"h (%"PRId32") (cell)", (UCELL)adr,
                                        (UCELL)c, c);
                }
            }
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    printf(format, display);
#pragma GCC diagnostic pop
    free(display);
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
    // FIXME: Use these next two!
    int exception = 0;
    CELL temp = 0;

    switch (no) {
    case c_TOD:
        {
            long value = single_arg(strtok(NULL, " "));

            if (debug)
                printf("Push %ld on to the data stack\n", value);
            if (range(SP, MEMORY + 1, "SP"))
                return;
            if (SP == 0) {
                printf("SP is 0h: no more stack items can be pushed\n");
                return;
            }
            PUSH(value);
        }
        break;
    case c_TOR:
        {
            long value = single_arg(strtok(NULL, " "));

            if (debug)
                printf("Push %ld on to the return stack\n", value);
            if (range(SP, MEMORY + 1, "RP"))
                return;
            if (RP == 0) {
                printf("RP is 0h: no more stack items can be pushed\n");
                return;
            }
            PUSH_RETURN(value);
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
                printf("Disassemble from %lX to %lX\n", (unsigned long)start, (unsigned long)end);
            disassemble((CELL)start / CELL_W, (CELL)end / CELL_W);
        }
        break;
    case c_DFROM:
        {
            if (debug)
                printf("Pop a number from the data stack and display it\n");
            if (range(SP, MEMORY, "SP"))
                return;
            CELL value = POP;
            printf("%"PRId32" (%"PRIX32"h)\n", value, (UCELL)value);
        }
        break;
    case c_DATA:
    case c_STACKS:
        if (debug)
            printf("Display the data stack\n");
        if (!range(RP, MEMORY + 1, "SP") &&
            !range(S0, MEMORY + 1, "S0")) {
            if (SP == S0)
                printf("Data stack empty\n");
            else if (SP > S0)
                printf("Data stack underflow\n");
            else
                show_data_stack();
        }
        if (no == c_STACKS)
            goto c_ret;
        break;
    case c_DEBUG:
        {
            char *arg = strtok(NULL, " ");
            debug = arg ? (single_arg(arg) != 0) : !debug;
        }
        break;
    case c_DUMP:
        {
            long start, end;
            double_arg(strtok(NULL, ""), &start, &end);
            if (range(start, MEMORY, "Address")) return;
            if (range(end, MEMORY, "Address")) return;
            if (start >= end) {
                printf("Start address must be less than end address\n");
                return;
            }
            if (debug)
                printf("Dump memory from %lX to %lX\n", (unsigned long)start, (unsigned long)end);
            while (start < end) {
                printf("%08lXh: ", (unsigned long)start);
                for (int i = 0; i < 8 && start < end; i++, start++)
                    printf("%02X ", ((BYTE *)M0)[start]);
                putchar('\n');
            }
        }
        break;
    case c_FROM:
        {
            char *arg = strtok(NULL, " ");
            if (arg != NULL) {
                long adr = single_arg(arg);
                if (range(adr, MEMORY, "EP"))
                    return;
                if (adr & 3) {
                    printf("Address must be cell-aligned\n");
                    break;
                }
                if (debug)
                    printf("Set EP to %lXh\n", (unsigned long)adr);
                EP = adr;
            }
            if (debug)
                printf("Perform NEXT\n");
            NEXT;
        }
        break;
    case c_INITIALISE:
    case c_LOAD:
        {
            if (debug)
                printf("Initialise Beetle\n");
            memset(M0, 0, memory_size);
            memset(count, 0, 256 * sizeof(long));
            init_beetle(M0, memory_size, 16);
            S0 = SP;
            R0 = RP;
            *THROW = 0;
            A = 0;
            if (no == c_LOAD)
                goto c_load;
            break;
        }
 c_load:
        {
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
                printf("Load binary image %s to address %lX\n", file, (unsigned long)adr);
            ret = load_object(handle, adr);
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
            default:
                break;
            }
        }
        break;
    case c_QUIT:
        exit(0);
    case c_REGISTERS:
        do_registers();
        break;
    case c_RFROM:
        {
            if (debug)
                printf("Pop a number from the return stack and display it\n");
            if (range(RP, MEMORY, "RP")) return;
            CELL value = POP_RETURN;
            printf("%"PRIX32"h (%"PRId32")\n", (UCELL)value, value);
        }
        break;
    c_ret:
    case c_RETURN:
        if (debug)
            printf("Display the return stack\n");
        if (range(RP, MEMORY + 1, "RP"))
            return;
        if (range(R0, MEMORY + 1, "R0"))
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
            CELL ret = -260;

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
                    while ((unsigned long)EP != limit && ret == -260) {
                        ret = single_step();
                        if (no == c_TRACE) do_registers();
                        count[I]++;
                    }
                    if (ret != 0)
                        printf("HALT code %"PRId32" was returned at EP = %Xh\n",
                               ret, EP);
                } else {
                    limit = single_arg(arg);
                    if (debug)
                        printf("STEP for %lu instructions\n", limit);
                    for (i = 0; i < limit && ret == -260; i++) {
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
                printf("Save memory to file %s from %lX to %lX\n", file,
                       (unsigned long)start, (unsigned long)end);
            ret = save_object(handle, start, (UCELL)((end - start) / CELL_W));
            fclose(handle);

            switch (ret) {
            case -1:
                printf("Address out of range or save area extends "
                       "beyond MEMORY\n");
                break;
            case -3:
                printf("Error while saving module\n");
                break;
            default:
                break;
            }
        }
        break;
    default: /* This cannot happen */
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
        int result = system(input + 1);
        if (result == -1)
            printf("Could not run command\n");
        else if (result != 0 && WIFEXITED(result))
            printf("Command exited with value %d\n", WEXITSTATUS(result));
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
        if (ass)
            do_ass(token);
        else
            do_display(token, "%s\n");
    }
}


static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void die(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stderr, "beetle: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    exit(1);
}

/* Options table */
struct option longopts[] = {
#define O(longname, shortname, arg, argstring, docstring) \
  {longname, arg, NULL, shortname},
#define A(argstring, docstring)
#include "tbl_opts.h"
#undef O
#undef A
  {0, 0, 0, 0}
};

#define BEETLE_VERSION_STRING "Beetle shell (C Beetle release "PACKAGE_VERSION")"
#define BEETLE_COPYRIGHT_STRING "(c) Reuben Thomas 1995-2018"

static void usage(void)
{
    char *shortopt, *buf;
    printf ("Usage: %s [OPTION...] [FILENAME ARGUMENT...]\n"
            "\n"
            "Run " PACKAGE_NAME ".\n"
            "\n",
            program_name);
#define O(longname, shortname, arg, argstring, docstring)               \
    shortopt = xasprintf(", -%c", shortname);                           \
    buf = xasprintf("--%s%s %s", longname, shortname ? shortopt : "", argstring); \
    printf("%-24s%s\n", buf, docstring);
#define A(argstring, docstring)                 \
    printf("%-24s%s\n", argstring, docstring);
#include "tbl_opts.h"
#undef O
#undef A
    printf("\n"
           "Report bugs to " PACKAGE_BUGREPORT ".\n");
}

int main(int argc, char *argv[])
{
    char input[MAXLEN], *nl;

    set_program_name(argv[0]);

    // Options string starts with '+' to stop option processing at first non-option, then
    // leading ':' so as to return ':' for a missing arg, not '?'
    for (;;) {
        int this_optind = optind ? optind : 1, longindex;
        int c = getopt_long(argc, argv, "+:m:", longopts, &longindex);

        if (c == -1)
            break;
        else if (c == ':')
            die("option '%s' requires an argument", argv[this_optind]);
        else if (c == '?')
            die("unrecognised option '%s'\nTry '%s --help' for more information.", argv[this_optind], program_name);
        else switch (longindex) {
            case 0:
                {
                    char *endptr;
                    errno = 0;
                    MEMORY = strtol(optarg, &endptr, 10);
                    if (*optarg == '\0' || *endptr != '\0' || MEMORY <= 0 || MEMORY > MAX_MEMORY)
                        die("memory size must be a positive number up to %"PRIu32, (UCELL)MAX_MEMORY);
                    break;
                }
            case 1:
                debug = true;
                break;
            case 2:
                usage();
                exit(EXIT_SUCCESS);
            case 3:
                printf(PACKAGE_NAME " " VERSION "\n"
                       BEETLE_COPYRIGHT_STRING "\n"
                       PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
                       "You may redistribute copies of " PACKAGE_NAME "\n"
                       "under the terms of the GNU General Public License.\n"
                       "For more information about these matters, see the file named COPYING.\n");
                exit(EXIT_SUCCESS);
            default:
                break;
            }
    }

    CELL *mem;
    if ((mem = (CELL *)calloc(memory_size, sizeof(CELL))) == NULL)
        die("could not allocate %"PRIu32" cells of memory", memory_size);

    init_beetle(mem, memory_size, 16);
    S0 = SP;
    R0 = RP;
    *THROW = 0;
    A = 0;

    argc -= optind;
    if (argc >= 1) {
        if (!register_args(argc, argv + optind))
            die("could not allocate memory to map command-line arguments");
        FILE *handle = fopen(argv[optind], "r");
        if (handle == NULL)
            die("cannot not open file %s", argv[1]);
        int ret = load_object(handle, 16);
        fclose(handle);
        if (ret != 0)
            die("could not read file %s", argv[1]);

        return run();
    }

    printf("%s\n%s\n\n", BEETLE_VERSION_STRING, BEETLE_COPYRIGHT_STRING);

    while (1) {
        if (setjmp(env) == 0) {
            printf(">");
            if (fgets(input, MAXLEN, stdin) == NULL) {
                if (feof(stdin)) {
                    putchar('\n'); // Empty line after prompt
                    exit(EXIT_SUCCESS);
                }
                die("input error");
            }
            if ((nl = strrchr(input, '\n')))
                *nl = '\0';
            parse(input);
        }
    }
}
