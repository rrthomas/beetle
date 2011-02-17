dnl Process this file with autoconf to produce a configure script

AC_INIT(beetle, 4, rrt@sc3d.org)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl Check for programs
AC_PROG_CC
AC_PROG_RANLIB

dnl Extra CFLAGS if we have gcc
if test "$GCC" = yes; then
    CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic"
fi

dnl Check features
AC_C_BIGENDIAN

dnl Check for header files
AC_HEADER_STDC

dnl Generate output files
AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_FILES(Makefile src/Makefile src/tests/Makefile)
AC_OUTPUT
