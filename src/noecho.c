/* NOECHO_BSD.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 02apr95 Adapted from Martin Richards's code.
    0.01 06jul04 Added System V support (also adapted from Martin's code).

    Reuben Thomas


    void init_keyb(void)  initialises the keyboard interface.
    void restore_keyb(void) restores the keyboard to its original state.

*/


#ifdef bsd

#define BSD_COMP
#include <stdio.h>
#include <sys/ioctl.h>
#include <sgtty.h>


void init_keyb(void)
{
    struct sgttyb ttyb;

    ioctl(0, TIOCGETP, &ttyb);
    ttyb.sg_flags = CBREAK + EVENP + ODDP + CRMOD;
    ioctl(0, TIOCSETP, &ttyb);
}

void restore_keyb(void)
{
    struct sgttyb ttyb;
    ioctl(0, TIOCGETP, &ttyb);
    ttyb.sg_flags = ECHO + EVENP + ODDP + CRMOD;
    ioctl(0, TIOCSETP, &ttyb);
}


#elif defined(sysv)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>


/* Use this variable to remember original terminal attributes */
struct termios saved_stdin;
struct termios saved_stdout;

int init_keyb(void)
{
    struct termios tattr;

    if (!isatty(STDIN_FILENO)) return -1;

    /* Save the terminal attributes so we can restore them later */
    tcgetattr(STDIN_FILENO, &saved_stdin);

    /* Set the funny terminal modes */
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO);   /* Clear ICANON and ECHO */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);

    setbuf(stdout, NULL);

    return 0;
}

int restore_keyb(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_stdin);
    return 0;
}

int getch(void)
{
    char ch;

    read(STDIN_FILENO, &ch, 1);
    return ch;
}

void putch(char ch)
{
    write(STDOUT_FILENO, &ch, 1);
}

#endif
