# Tests Makefile.am

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

LOG_COMPILER = $(srcdir)/run-test

LDADD = $(top_builddir)/src/libbeetle.la

check_PROGRAMS = $(TESTS) libt loadobjt

TESTS = aritmtit brancht comparet epfetcht doloopt initialt literalt \
	logict memoryt stackt stept runt saveobjt exceptst linkt
TESTS_ENVIRONMENT = \
	if test -n "$(VALGRIND)"; then export VALGRIND='$(VALGRIND)'; fi; \
	export LIBTOOL=$(top_builddir)/libtool;

do_loadobjt: loadobjt
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(builddir)/loadobjt $(srcdir)

do_libt: libt
	$(TESTS_ENVIRONMENT) echo "Reverse this text" | $(LOG_COMPILER) $(builddir)/libt

check-local: do_loadobjt do_libt

EXTRA_DIST = run-test btests.h badobj1 badobj2 badobj3 badobj4 testobj1 testobj2
