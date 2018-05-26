// A user interface for Beetle.
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

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
#include "beetle_aux.h"
#include "opcodes.h"
#include "debug.h"


#define DEFAULT_MEMORY 1048576 // Default size of Beetle's memory in cells (4Mb)
#define MAX_MEMORY 536870912 // Maximum size of memory in cells (2Gb)
static UCELL memory_size = DEFAULT_MEMORY; // Size of Beetle's memory in cells
CELL *memory;

#define MAXLEN 80   // Maximum input line length

static jmp_buf env;

static bool debug_on_error = false;

static const char *command[] = {
#define C(cmd) #cmd,
#include "tbl_commands.h"
#undef C
};
enum commands {
#define C(cmd) c_##cmd,
#include "tbl_commands.h"
#undef C
};
static int commands = sizeof(command) / sizeof(*command);

static const char *regist[] = {
#define R(reg) #reg,
#include "tbl_registers.h"
#undef R
};
enum registers {
#define R(reg) r_##reg,
#include "tbl_registers.h"
#undef R
};
static int registers = sizeof(regist) / sizeof(*regist);

static long count[256];


static void check_valid(UCELL adr, const char *quantity)
{
    if (native_address(adr, false) == NULL) {
        printf("%s is invalid\n", quantity);
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

static void check_range(UCELL start, UCELL end, const char *quantity)
{
    check_valid(start, quantity);
    check_valid(end, quantity);
    if (start >= end) {
        printf("Start address must be less than end address\n");
        longjmp(env, 1);
    }
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
    if (s[0] == '$')
        n = strtol(s + 1, &endp, 16);
    else
        n = strtol(s, &endp, 10);

    if (endp != &s[len]) {
        printf("Invalid number\n");
        longjmp(env, 1);
    }

    return n;
}

static void double_arg(char *s, long *start, long *end)
{
    char *token, copy[MAXLEN];
    if (s == NULL || (token = strtok(strcpy(copy, s), " +")) == NULL) {
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
    return load_op(o & 0xfe);
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


static void reinit(void)
{
    memset(count, 0, 256 * sizeof(long));
    init_beetle(memory, memory_size);
}


static int save_object(FILE *file, UCELL address, UCELL length)
{
    uint8_t *ptr = native_address_range_in_one_area(address, length, false);
    if (!IS_ALIGNED(address) || ptr == NULL)
        return -1;

    if (fputs("BEETLE", file) == EOF ||
        putc('\0', file) == EOF ||
        putc((char)ENDISM, file) == EOF ||
        fwrite(&length, CELL_W, 1, file) != 1 ||
        fwrite(ptr, CELL_W, length, file) != length)
        return -3;

    return 0;
}


static void do_assign(char *token)
{
    char *number = strtok(NULL, " ");
    int len = strlen(number);
    long value;
    bool byte = false;

    upper(number);
    if (number[0] == 'O') {
        value = toass(number + 1);
        if (value == 0x7f) {
            printf("Invalid opcode\n");
            return;
        }
        byte = true;
    } else {
        value = single_arg(number);
        if (number[0] == '$' && len < 4)
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
            NOT_ADDRESS = value;
            break;
        case r_BAD:
            BAD = value;
            break;
        case r_CHECKED:
        case r_ENDISM:
        case r_MEMORY:
            printf("Can't assign to %s\n", regist[no]);
            break;
        case r_EP:
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
            RP = value;
            break;
        case r_R0:
            R0 = value;
            break;
        case r_SP:
            SP = value;
            break;
        case r_S0:
            S0 = value;
            break;
        case r_THROW:
            THROW = value;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                check_valid(adr, "Address");
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
            display = xasprintf("'THROW = %"PRIX32"h (%"PRIu32")", THROW, THROW);
            break;
        default:
            {
                CELL adr = (CELL)single_arg(token);

                check_valid(adr, "Address");
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
            PUSH(value);
        }
        break;
    case c_TOR:
        {
            long value = single_arg(strtok(NULL, " "));
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
            check_aligned(start, "Address");
            check_aligned(end, "Address");
            check_range(start, end, "Address");
            disassemble((UCELL)start, (UCELL)end);
        }
        break;
    case c_DFROM:
        {
            CELL value = POP;
            printf("%"PRId32" (%"PRIX32"h)\n", value, (UCELL)value);
        }
        break;
    case c_DATA:
    case c_STACKS:
        show_data_stack();
        if (no == c_STACKS)
            goto c_ret;
        break;
    case c_DUMP:
        {
            long start, end;
            double_arg(strtok(NULL, ""), &start, &end);
            check_range(start, end, "Address");
            while (start < end) {
                printf("%08lXh ", (unsigned long)start);
                const int chunk = 16;
                char ascii[chunk];
                for (int i = 0; i < chunk && start < end; i++) {
                    BYTE byte;
                    beetle_load_byte(start + i, &byte);
                    if (i % 8 == 0)
                        putchar(' ');
                    printf("%02X ", byte);
                    ascii[i] = isprint(byte) ? byte : '.';
                }
                start += chunk;
                printf(" |%.*s|\n", chunk, ascii);
            }
        }
        break;
    case c_FROM:
        {
            char *arg = strtok(NULL, " ");
            if (arg != NULL) {
                long adr = single_arg(arg);
                EP = adr;
            }
            NEXT;
        }
        break;
    case c_INITIALISE:
        reinit();
        break;
    case c_LOAD:
        {
            reinit();

            const char *file = strtok(NULL, " ");
            long adr = 0;
            char *arg = strtok(NULL, " ");
            if (arg != NULL)
                adr = single_arg(arg);

            FILE *handle = fopen(file, "rb");
            if (handle == NULL) {
                printf("Cannot open file %s\n", file);
                return;
            }
            int ret = load_object(handle, adr);
            fclose(handle);

            switch (ret) {
            case -1:
                printf("Address out of range or unaligned, or module too large\n");
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
            CELL value = POP_RETURN;
            printf("%"PRIX32"h (%"PRId32")\n", (UCELL)value, value);
        }
        break;
    c_ret:
    case c_RETURN:
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
                    check_valid(limit, "Address");
                    check_aligned(limit, "Address");
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
                printf("Save area contains an invalid address\n");
                break;
            case -3:
                printf("Error while saving module\n");
                break;
            default:
                break;
            }
        }
        break;
    default: // This cannot happen
        break;
    }

    switch (exception) {
    case -9:
        printf("Invalid address\n");
        break;
    case -23:
        printf("Address alignment exception\n");
        break;
    default:
    case 0:
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

    bool assign = false;
    if (i < strlen(input))
        assign = input[i] == '=';

    size_t no = search(token, command, commands);
    if (no != SIZE_MAX)
        do_command(no);
    else {
        if (assign)
            do_assign(token);
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

// Options table
struct option longopts[] = {
#define O(longname, shortname, arg, argstring, docstring) \
  {longname, arg, NULL, shortname},
#define A(argstring, docstring)
#define D(docstring)
#include "tbl_opts.h"
#undef O
#undef A
#undef D
  {0, 0, 0, 0}
};

#define BEETLE_VERSION_STRING "Beetle shell (C Beetle release "PACKAGE_VERSION")"
#define BEETLE_COPYRIGHT_STRING "(c) Reuben Thomas 1995-2018"

static void usage(void)
{
    char *shortopt, *buf;
    printf ("Usage: %s [OPTION...] [OBJECT-FILE ARGUMENT...]\n"
            "\n"
            "Run " PACKAGE_NAME ".\n"
            "\n",
            program_name);
#define O(longname, shortname, arg, argstring, docstring)               \
    shortopt = xasprintf(", -%c", shortname);                           \
    buf = xasprintf("--%s%s %s", longname, shortname ? shortopt : "", argstring); \
    printf("  %-26s%s\n", buf, docstring);
#define A(argstring, docstring)                 \
    printf("  %-26s%s\n", argstring, docstring);
#define D(text)                                 \
    printf(text "\n");
#include "tbl_opts.h"
#undef O
#undef A
#undef D
}

static CELL parse_memory_size(UCELL max)
{
    char *endptr;
    errno = 0;
    long size = (CELL)strtol(optarg, &endptr, 10);
    if (*optarg == '\0' || *endptr != '\0' || size <= 0 || (UCELL)size > max)
        die("memory size must be a positive number up to %"PRIu32, max);
    return size;
}

int main(int argc, char *argv[])
{
    char input[MAXLEN], *nl;

    set_program_name(argv[0]);

    // Options string starts with '+' to stop option processing at first non-option, then
    // leading ':' so as to return ':' for a missing arg, not '?'
    for (;;) {
        int this_optind = optind ? optind : 1, longindex = -1;
        int c = getopt_long(argc, argv, "+:dm:", longopts, &longindex);

        if (c == -1)
            break;
        else if (c == ':')
            die("option '%s' requires an argument", argv[this_optind]);
        else if (c == '?')
            die("unrecognised option '%s'\nTry '%s --help' for more information.", argv[this_optind], program_name);
        else if (c == 'm')
            longindex = 0;
        else if (c == 's')
            longindex = 1;

        switch (longindex) {
            case 0:
                memory_size = parse_memory_size((UCELL)MAX_MEMORY);
                break;
            case 1:
                HASHS = parse_memory_size((UCELL)MAX_STACK_SIZE);
                break;
            case 2:
                HASHR = parse_memory_size((UCELL)MAX_STACK_SIZE);
                break;
            case 3:
                debug_on_error = true;
                break;
            case 4:
                usage();
                exit(EXIT_SUCCESS);
            case 5:
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

    if ((memory = (CELL *)calloc(memory_size, CELL_W)) == NULL)
        die("could not allocate %"PRIu32" cells of memory", memory_size);
    reinit();

    argc -= optind;
    if (argc >= 1) {
        if (!register_args(argc, argv + optind))
            die("could not allocate memory to map command-line arguments");
        FILE *handle = fopen(argv[optind], "rb");
        if (handle == NULL)
            die("cannot not open file %s", argv[1]);
        int ret = load_object(handle, 0);
        fclose(handle);
        if (ret != 0)
            die("could not read file %s", argv[1]);

        int res = run();
        if (!debug_on_error || res >= 0)
            return res;
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
