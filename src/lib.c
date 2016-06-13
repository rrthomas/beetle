/* LIB.C

    (c) Reuben Thomas 1995-2016

    Beetle's standard library.

*/


#include "config.h"

#include <stdio.h>
#include "beetle.h"     /* main header */
#include "lib.h"        /* the header we're implementing */


#define CHECKA(x) // FIXME: import this from step.c (move to beetle.h), and use it in routines >= 4

#define PTRS 64
#define PTR_OK(p) ((p) >= 0 && (p) <= lastptr)

static void getstr(unsigned char *s, UCELL adr)
{
    int i;

    for (i = 0; *(M0 + FLIP(adr)) != 0; adr++)
        s[i++] = *(M0 + FLIP(adr));
    s[i] = '\0';
}

void lib(UCELL routine)
{
    static FILE *ptr[PTRS];
    static int lastptr = 0;

    switch (routine) {

    case 0: /* BL */
        CHECKA(SP - 1);
        *--SP = 32;
        break;

    case 1: /* CR */
        putchar('\n');
        break;

    case 2: /* EMIT */
        CHECKA(SP);
        putchar((BYTE)*SP);
        SP++;
        break;

    case 3: /* KEY */
        CHECKA(SP - 1);
        *--SP = (CELL)(getchar());
        break;

    case 4: /* OPEN-FILE */
        {
            int p = (lastptr == PTRS ? -1 : lastptr++);
            unsigned char file[256], perm[4];

            if (p == -1)
                *SP = -1;
            else {
                getstr(file, *((UCELL *)SP + 1));
                getstr(perm, *(UCELL *)SP);
                ptr[p] = fopen((char *)file, (char *)perm);
                *SP = 0;
                *(SP + 1) = p + 1;
            }
        }
        break;

    case 5: /* CLOSE-FILE */
        {
            int p = *SP - 1;
            if (!PTR_OK(p)) {
                *SP = -1;
                break;
            }
            *SP = fclose(ptr[p]);
            for (int i = p; i < lastptr; i++)
                ptr[i] = ptr[i + 1];
            lastptr--;
        }
        break;

    case 6: /* READ-FILE */
        {
            unsigned long i;
            int c = 0, p = *SP - 1;

            if (!PTR_OK(p)) {
                *++SP = -1;
                *(SP + 1) = 0;
                break;
            }
            for (i = 0; i < *((UCELL *)SP + 1) && c != EOF; ) {
                c = getc(ptr[p]);
                if (c != EOF)
                    *(M0 + FLIP(*((UCELL *)SP + 2) + i++)) = (BYTE)c;
            }
            *++SP = ferror(ptr[p]) ? -1 : 0;
            *((UCELL *)SP + 1) = (UCELL)i;
        }
        break;

    case 7: /* WRITE-FILE */
        {
            unsigned long i;
            int c = 0, p = *SP - 1;

            if (PTR_OK(p))
                for (i = 0; i < *((UCELL *)SP + 1); i++)
                    if ((c = fputc(*(M0 + FLIP(*((UCELL *)SP + 2) + i)),
                                   ptr[p])) == EOF)
                        break;
            else
                c = EOF;
            SP += 2;
            if (c != EOF)
                *SP = 0;
            else
                *SP = -1;
        }
        break;

    case 8: /* FILE-POSITION */
        {
            int p = *SP - 1;
            if (!PTR_OK(p)) {
                *(SP -= 2) = -1;
                break;
            }

            // FIXME: split long into two CELLs properly
            long res = ftell(ptr[p]);
            *((UCELL *)SP--) = (UCELL)res;
            *SP-- = 0; // Extend number to double
            if (res != -1)
                *SP = 0;
            else
                *SP = -1;
        }
        break;

    case 9: /* REPOSITION-FILE */
        {
            int p = *SP - 1;
            if (!PTR_OK(p)) {
                *(SP + 2) = -1;
                break;
            }
            // FIXME: Read from two CELLs properly
            int res = fseek(ptr[p], *((UCELL *)SP + 2), SEEK_SET);

            *(SP += 2) = (UCELL)res;
        }
        break;

    case 10: /* FLUSH-FILE */
        {
            int p = *SP - 1;
            if (!PTR_OK(p)) {
                *SP = -1;
                break;
            }

            int res = fflush(ptr[p]);
            if (res != EOF)
                *SP = 0;
            else
                *SP = -1;
        }
        break;

    case 11: /* RENAME-FILE */
        {
            int res;
            unsigned char from[256], to[256];

            getstr(from, *((UCELL *)SP + 1));
            getstr(to, *(UCELL *)SP++);
            res = rename((char *)from, (char *)to);

            if (res != 0)
                *SP = -1;
            else
                *SP = 0;
        }
        break;

    case 12: /* DELETE-FILE */
        {
            int res;
            unsigned char file[256];

            getstr(file, *(UCELL *)SP);
            res = remove((char *)file);

            if (res != 0)
                *SP = -1;
            else
                *SP = 0;
        }
        break;

    }
}
