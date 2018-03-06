# Tests Makefile.am

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

LOG_COMPILER = $(srcdir)/run-test

LDADD = $(top_builddir)/src/libbeetle.la

check_PROGRAMS = $(TESTS) loadobjt

TESTS = aritmtit brancht comparet epfetcht doloopt initialt libt literalt \
	logict memoryt stackt stept runt exceptst linkt
TESTS_ENVIRONMENT = \
	if test -n "$(VALGRIND)"; then export VALGRIND='$(VALGRIND)'; fi; \
	export LIBTOOL=$(top_builddir)/libtool;

do_loadobjt: loadobjt
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(builddir)/loadobjt $(srcdir)

check-local: do_loadobjt

EXTRA_DIST = run-test btests.h badobj1 badobj2 badobj3 badobj4 testobj1 testobj2
