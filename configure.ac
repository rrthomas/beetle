# configure.ac for Beetle
# Process this file with autoconf to produce a configure script
#
# (c) Reuben Thomas 2011-2023
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AC_PREREQ([2.71])
AC_INIT([Beetle],[3.0.2],[rrt@sc3d.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Check for programs
AC_PROG_CC
gl_EARLY
AM_PROG_AR
LT_INIT
AC_PATH_PROG(LATEXMK, latexmk, true)
AM_CONDITIONAL([HAVE_LATEXMK], [test "$ac_cv_path_LATEXMK" != "true"])

# help2man
# Set a value even if not found, so that an invocation via build-aux/missing works
AC_PATH_PROG([HELP2MAN], [help2man], [help2man])

# Optimization
AX_CC_MAXOPT

# Readline wrapper
AC_PATH_PROG(RLWRAP, rlwrap)
AM_CONDITIONAL([HAVE_RLWRAP], [test -n "$ac_cv_path_RLWRAP"])

# beetle-mijit
AC_ARG_WITH([mijit],
  [AS_HELP_STRING([--with-mijit], [use mijit-beetle JIT compiler])],
  [case $withval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $withval for mijit option]) ;;
   esac
   with_mijit=$withval],
  [with_mijit=no]
)
if test "$with_mijit" = yes; then
  AC_PATH_PROG(CARGO, cargo)
  if test -z "$ac_cv_path_CARGO"; then
    AC_MSG_ERROR(cargo not found)
  fi
  AC_DEFINE(HAVE_MIJIT, 1, [Whether we are using Mijit.])
fi
AM_CONDITIONAL([HAVE_MIJIT], [test "$with_mijit" = yes])

# Extra warnings with GCC
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--disable-gcc-warnings],
                  [turn off lots of GCC warnings])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=yes]
)
if test "$gl_gcc_warnings" = yes; then
  # Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings

  gl_MANYWARN_ALL_GCC([warnings])

  # Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])
gl_INIT

# Check features
AC_C_BIGENDIAN

# Generate output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
        lib/Makefile
        src/Makefile
        tests/Makefile
        doc/Makefile
        doc/cbeetle.tex
        doc/shell.tex
])
AC_OUTPUT
