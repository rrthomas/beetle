/* NOECHO.C

    (c) Martin Richards 2004 (and certainly earlier)
    (c) Reuben Thomas 1995-2018

    void init_keyb(void)  initialises the keyboard interface.
    void restore_keyb(void) restores the keyboard to its original state.

*/


#include "config.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#include "noecho.h"


#ifdef HAVE_TERMIOS_H
/* Use this variable to remember original terminal attributes */
static struct termios saved_stdin;
#endif

bool save_called = false;

int init_keyb(void)
{
#ifdef HAVE_TERMIOS_H
    struct termios tattr;

    if (!isatty(STDIN_FILENO)) return -1;

    /* Save the terminal attributes so we can restore them later */
    tcgetattr(STDIN_FILENO, &saved_stdin);

    /* Set the funny terminal modes */
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO);   /* Clear ICANON and ECHO */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
#endif

    setbuf(stdout, NULL);

    save_called = true;

    return 0;
}

int restore_keyb(void)
{
    if (save_called) {
#ifdef HAVE_TERMIOS_H
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_stdin);
#endif

        setvbuf(stdout, NULL, _IOLBF, 0);
    }

    return 0;
}
