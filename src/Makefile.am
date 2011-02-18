# Source Makefile.am

SUBDIRS = tests

noinst_LIBRARIES = libbeetle.a
libbeetle_a_SOURCES = storage.c tests.c step.c run.c lib.c debug.c loadobj.c saveobj.c noecho.c \
	beetle.h debug.h lib.h noecho.h opcodes.h tests.h

bin_PROGRAMS = beetle
beetle_LDADD = libbeetle.a
beetle_SOURCES = beetle.c

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_a_SOURCES) $(beetle_SOURCES)

DIST_EXTRA = ARMBeetle.bas beetle,ff8
