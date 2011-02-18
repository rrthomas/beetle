dnl Process this file with autoconf to produce a configure script

AC_INIT(beetle, 4, rrt@sc3d.org)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl Check for programs
AC_PROG_CC
AX_CFLAGS_WARN_ALL
AC_PROG_RANLIB

dnl Valgrind
AX_WITH_PROG(VALGRIND, valgrind)
if test -n "$VALGRIND"; then
  VALGRIND="$VALGRIND --error-exitcode=1 --leak-check=full"
fi

dnl Extra CFLAGS if we have gcc
if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -W -Wmissing-prototypes -Wstrict-prototypes -D_FORTIFY_SOURCE=2 -pedantic"
fi

dnl Check features
AC_C_BIGENDIAN

dnl Check for header files
AC_HEADER_STDC

dnl Generate output files
AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_FILES(Makefile src/Makefile src/tests/Makefile)
AC_OUTPUT
