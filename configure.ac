dnl Process this file with autoconf to produce a configure script

AC_INIT(beetle, 4, rrt@sc3d.org)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Check for programs
AC_PROG_CC_C99
AX_CFLAGS_WARN_ALL
if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -W -Wmissing-prototypes -Wstrict-prototypes -D_FORTIFY_SOURCE=2 -pedantic"
fi
gl_EARLY
AM_PROG_AR
AC_PROG_RANLIB
AX_WITH_PROG(RUBBER, rubber, true)

dnl Valgrind
AX_WITH_PROG(VALGRIND, valgrind)
if test -n "$VALGRIND"; then
  VALGRIND="$VALGRIND -q --error-exitcode=1 --leak-check=full"
fi

dnl Check for header files
AC_HEADER_STDC
gl_INIT

dnl Check features
AC_C_BIGENDIAN

dnl Generate output files
AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_FILES(Makefile lib/Makefile doc/Makefile src/Makefile src/tests/Makefile)
AC_OUTPUT
