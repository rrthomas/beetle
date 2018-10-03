# Tests Makefile.am
#
# (c) Reuben Thomas 2011-2018
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
# RISK.

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

LOG_COMPILER = $(srcdir)/run-test

LDADD = $(top_builddir)/src/lib@PACKAGE@.la

check_PROGRAMS = $(TESTS) load_object

TESTS = arithmetic branch comparison doloop init literals logic memory \
	registers stack single_step run exceptions link extra_instructions
TESTS_ENVIRONMENT = \
	if test -n "$(VALGRIND)"; then export VALGRIND='$(VALGRIND)'; fi; \
	export LIBTOOL=$(top_builddir)/libtool;

test-parser:
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(top_builddir)/src/@PACKAGE@$(EXEEXT) < $(srcdir)/hello.txt && \
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(top_builddir)/src/@PACKAGE@$(EXEEXT) ./hello.obj > hello.output && \
	diff hello.output $(srcdir)/hello.correct

do_load_object: load_object test-parser
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(builddir)/load_object $(srcdir)

check-local: do_load_object test-parser

EXTRA_DIST = run-test tests.h badobj1 badobj2 badobj3 badobj4 testobj1 testobj2 hello.txt hello.correct

DISTCLEANFILES = hello.obj hello.output
