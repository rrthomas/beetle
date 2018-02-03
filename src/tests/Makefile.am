# Tests Makefile.am

AM_CPPFLAGS = -I$(srcdir)/.. -I$(top_builddir)/lib -I$(top_srcdir)/lib

LDADD = $(top_builddir)/src/libbeetle.a

check_PROGRAMS = $(TESTS) libt loadobjt

TESTS = aritmtit brancht comparet createt doloopt initialt literalt \
	logict memoryt stackt stept runt saveobjt exceptst linkt
TESTS_ENVIRONMENT = $(VALGRIND)

do_loadobjt: loadobjt
	$(TESTS_ENVIRONMENT) $(builddir)/loadobjt $(srcdir)

do_libt: libt
	echo "Reverse this text" | $(TESTS_ENVIRONMENT) $(builddir)/libt

check-local: do_loadobjt do_libt

EXTRA_DIST = btests.h badobj1 badobj2 badobj3 badobj4 testobj1 testobj2
