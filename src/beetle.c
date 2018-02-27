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


#define DEFAULT_MEMORY 1048576 // Default size of Beetle's memory in cells (4Mb)
#define MAX_MEMORY 536870912 // Maximum size of memory in cells (2Gb)
static UCELL memory_size = DEFAULT_MEMORY; // Size of Beetle's memory in cells

#define MAXLEN 80   // Maximum input line length

static jmp_buf env;

static const char *command[] = {
    ">D", ">R", "COUNTS", "DISASSEMBLE", "D>", "DATA",
    "DUMP", "FROM", "INITIALISE", "LOAD", "QUIT", "REGISTERS", "R>",
    "RETURN", "RUN", "STEP", "SAVE", "STACKS", "TRACE"
};
enum commands { c_TOD, c_TOR, c_COUNTS, c_DISASSEMBLE, c_DFROM, c_DATA,
    c_DUMP, c_FROM, c_INITIALISE, c_LOAD, c_QUIT, c_REGISTERS, c_RFROM,
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


static void check_in_range(UCELL adr, UCELL limit, const char *quantity)
{
    if (adr >= limit) {
        printf("%s must be in the range {0..%"PRIX32"h} ({0..%"PRIu32"})\n",
               quantity, (UCELL)limit, (UCELL)limit);
        longjmp(env, 1);
    }
}

static void check_aligned(UCELL adr, const char *quantity)
{
    if (!IS_ALIGNED(adr)) {
        printf("%s must be cell-aligned\n", quantity);
        longjmp(env, 1);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
static void check_aligned_in_range(UCELL adr, UCELL limit, const char *quantity)
{
    check_in_range(adr, limit, quantity);
    check_aligned(adr, quantity);
}
#pragma GCC diagnostic pop

static void check_range(UCELL start, UCELL end, UCELL limit, const char *quantity)
{
    check_in_range(start, limit, quantity);
    check_in_range(end, limit, quantity);
    if (start >= end) {
        printf("Start address must be less than end address\n");
        longjmp(env, 1);
    }
}

static void check_aligned_range(UCELL start, UCELL end, UCELL limit, const char *quantity)
{
    check_aligned(start, quantity);
    check_aligned(end, quantity);
    check_range(start, end, limit, quantity);
}

static void upper(char *s)
{
    size_t len = strlen(s);

    for (size_t i = 0; i < len; i++, s++)
        *s = toupper(*s);
}

static size_t search(const char *token, const char *list[], size_t entries)
{
    size_t len = strlen(token);

    for (size_t i = 0; i < entries; i++)
        if (strncmp(token, list[i], len) == 0)
            return i;

    return SIZE_MAX;
}


static long single_arg(const char *s)
{
    if (s == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }
    size_t len = strlen(s);

    long n;
    char *endp;
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
    if (s == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }

    char *token, copy[MAXLEN];
    strcpy(copy, s);
    if ((token = strtok(copy, " +")) == NULL) {
        printf("Too few arguments\n");
        longjmp(env, 1);
    }

    size_t i;
    for (i = strlen(token); s[i] == ' ' && i < strlen(s); i++)
        ;

    bool plus = false;
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

static void disassemble(UCELL start, UCELL end)
{
    for (UCELL p = start; p < end; ) {
        printf("%08"PRIX32"h: ", p);
        CELL a;
        beetle_load_cell(p, &a);
        p += CELL_W;

        do {
            BYTE i = (BYTE)a;
            ARSHIFT(a, 8);
            const char *token = disass(i);
            if (strcmp(token, ""))
                printf("%s", token);
            else
                printf("Undefined instruction");

            if (load_op(i)) {
                CELL lit;
                beetle_load_cell(p, &lit);
                if (i != O_LITERAL)
                    printf(" %"PRIX32"h", (UCELL)lit);
                else
                    printf(" %"PRId32" (%"PRIX32"h)", lit, (UCELL)lit);
                p += CELL_W;
            } else {
                if (imm_op(i)) {
                    if (i != O_LITERALI)
                        printf(" %"PRIX32"h", p + a * CELL_W);
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
    char *number = strtok(NULL, " ");
    int len = strlen(number);
    long value;
    bool byte = false;

    upper(number);
    if (number[0] == 'O') {
        value = toass(number + 1);
        if (value == 0xfe) {
            printf("Invalid opcode\n");
            return;
        }
        byte = true;
    } else {
        value = single_arg(number);
        if (number[len - 1] == 'H' && len < 4)
            byte = true;
        else {
            if ((unsigned long)value < 10)
                byte = len == 1;
            else if ((unsigned long)value < 100)
                byte = len == 2;
            else if ((unsigned long)value < 255)
                byte = len == 3;
        }
    }

    int no = search(token, regist, registers);
    switch (no) {
        case r_A:
            A = value;
            break;
        case r_NOT_ADDRESS:
            M0[3] = NOT_ADDRESS = value;
            break;
        case r_BAD:
            M0[2] = BAD = value;
            break;
        case r_CHECKED:
        case r_ENDISM:
        case r_M0:
        case r_MEMORY:
            printf("Can't assign to %s\n", regist[no]);
            break;
        case r_EP:
            check_aligned_in_range(value, MEMORY, "EP");
            EP = value;
            break;
        case r_I:
            if (value > 255) {
                printf("I must be assigned a byte\n");
                break;
            }
            I = value;
            break;
        case r_RP:
            check_aligned_in_range(value, MEMORY + 1, "RP");
            RP = value;
            break;
        case r_R0:
            check_aligned_in_range(value, MEMORY + 1, "R0");
            R0 = value;
            break;
        case r_SP:
            check_aligned_in_range(value, MEMORY + 1, "SP");
            SP = value;
            break;
        case r_S0:
            check_aligned_in_range(value, MEMORY + 1, "S0");
            S0 = value;
            break;
        case r_THROW:
            check_aligned_in_range(value, MEMORY, "'THROW");
            *THROW = value;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                check_in_range(adr, MEMORY, "Address");
                if (!IS_ALIGNED(adr) && byte == false) {
                    printf("Only a byte can be assigned to an unaligned "
                        "address\n");
                    return;
                }
                if (byte == true)
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

                check_in_range(adr, MEMORY, "Address");
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
    do_display("EP", "%-25s");
    do_display("I", "%-22s");
    do_display("A", "%-16s\n");
}

static void do_command(int no)
{
    int exception = 0;
    CELL temp = 0;

    switch (no) {
    case c_TOD:
        {
            long value = single_arg(strtok(NULL, " "));

            check_in_range(SP, MEMORY + 1, "SP");
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

            check_in_range(SP, MEMORY + 1, "RP");
            if (RP == 0) {
                printf("RP is 0h: no more stack items can be pushed\n");
                return;
            }
            PUSH_RETURN(value);
        }
        break;
    case c_COUNTS:
        {
            for (int i = 0; i < 92; i++) {
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
            check_aligned_range(start, end, MEMORY, "Address");
            disassemble((UCELL)start, (UCELL)end);
        }
        break;
    case c_DFROM:
        {
            check_in_range(SP, MEMORY, "SP");
            CELL value = POP;
            printf("%"PRId32" (%"PRIX32"h)\n", value, (UCELL)value);
        }
        break;
    case c_DATA:
    case c_STACKS:
        check_in_range(RP, MEMORY + 1, "SP");
        check_in_range(S0, MEMORY + 1, "S0");
        if (SP == S0)
            printf("Data stack empty\n");
        else if (SP > S0)
            printf("Data stack underflow\n");
        else
            show_data_stack();
        if (no == c_STACKS)
            goto c_ret;
        break;
    case c_DUMP:
        {
            long start, end;
            double_arg(strtok(NULL, ""), &start, &end);
            check_range(start, end, MEMORY, "Address");
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
                check_aligned_in_range(adr, MEMORY, "EP");
                EP = adr;
            }
            NEXT;
        }
        break;
    case c_INITIALISE:
    case c_LOAD:
        {
            memset(M0, 0, memory_size);
            memset(count, 0, 256 * sizeof(long));
            init_beetle(M0, memory_size, 16);
            S0 = SP;
            R0 = RP;
            *THROW = 0;
            A = 0;
            if (no != c_LOAD)
                break;

            const char *file = strtok(NULL, " ");
            long adr = single_arg(strtok(NULL, " "));

            FILE *handle;
            if ((handle = fopen(file, "rb")) == NULL) {
                printf("Cannot open file %s\n", file);
                return;
            }
            int ret = load_object(handle, adr);
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
            check_in_range(RP, MEMORY, "RP");
            CELL value = POP_RETURN;
            printf("%"PRIX32"h (%"PRId32")\n", (UCELL)value, value);
        }
        break;
    c_ret:
    case c_RETURN:
        check_in_range(RP, MEMORY + 1, "RP");
        check_in_range(R0, MEMORY + 1, "R0");
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
        printf("HALT code %"PRId32" was returned\n", run());
        break;
    case c_STEP:
    case c_TRACE:
        {
            char *arg = strtok(NULL, " ");
            CELL ret = -260;

            if (arg == NULL) {
                if ((ret = single_step()))
                    printf("HALT code %"PRId32" was returned\n", ret);
                if (no == c_TRACE) do_registers();
                count[I]++;
            } else {
                upper(arg);
                if (strcmp(arg, "TO") == 0) {
                    unsigned long limit = single_arg(strtok(NULL, ""));
                    check_aligned_in_range(limit, MEMORY, "Address");
                    while ((unsigned long)EP != limit && ret == -260) {
                        ret = single_step();
                        if (no == c_TRACE) do_registers();
                        count[I]++;
                    }
                    if (ret != 0)
                        printf("HALT code %"PRId32" was returned at EP = %Xh\n",
                               ret, EP);
                } else {
                    unsigned long limit = single_arg(arg), i;
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
            double_arg(strtok(NULL, ""), &start, &end);

            FILE *handle;
            if ((handle = fopen(file, "wb")) == NULL) {
                printf("Cannot open file %s\n", file);
                return;
            }
            int ret = save_object(handle, start, (UCELL)((end - start) / CELL_W));
            fclose(handle);

            switch (ret) {
            case -1:
                printf("Invalid address or save area extends beyond MEMORY\n");
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
    if (input[0] == '!') {
        int result = system(input + 1);
        if (result == -1)
            printf("Could not run command\n");
        else if (result != 0 && WIFEXITED(result))
            printf("Command exited with value %d\n", WEXITSTATUS(result));
        return;
    }

    char copy[MAXLEN];
    strcpy(copy, input);
    char *token = strtok(copy, strchr(copy, '=') == NULL ? " " : "=");
    if (token == NULL || strlen(token) == 0) return;
    upper(token);

    size_t i;
    for (i = strlen(token); input[i] == ' ' && i < strlen(input); i++)
        ;

    bool ass = false;
    if (i < strlen(input))
        ass = input[i] == '=';

    size_t no = search(token, command, commands);
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
