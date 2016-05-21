# Source Makefile.am

SUBDIRS = tests

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
LDADD = $(top_builddir)/lib/libgnu.a

noinst_LIBRARIES = libbeetle.a
libbeetle_a_SOURCES = storage.c tests.c step.c run.c lib.c debug.c loadobj.c saveobj.c noecho.c \
	beetle.h debug.h lib.h noecho.h opcodes.h tests.h

bin_PROGRAMS = beetle
beetle_LDADD = libbeetle.a
beetle_SOURCES = beetle.c

check-local: libbeetle.a

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_a_SOURCES) $(beetle_SOURCES)

EXTRA_DIST = ARMbeetle,ffb
