# Tests Makefile.am

AM_CPPFLAGS = -I$(srcdir)/..

LDADD = $(top_builddir)/src/libbeetle.a

check_PROGRAMS = aritmtit brancht comparet createt doloopt initialt libt literalt \
	logict memoryt stackt stept runt loadobjt saveobjt exceptst linkt

do_loadobjt: loadobjt
	$(builddir)/loadobjt $(srcdir)

do_libt: libt
	echo "Reverse this text" | $(builddir)/libt

TESTS = aritmtit brancht comparet createt doloopt initialt literalt \
	logict memoryt stackt stept runt saveobjt exceptst linkt

check-local: do_loadobjt do_libt

EXTRA_DIST = btests.h badobj1 badobj2 badobj3 badobj4 testobj1 testobj2
