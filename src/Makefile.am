# Source Makefile.am

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbeetle.la
libbeetle_la_SOURCES = storage.c step.c run.c debug.c loadobj.c saveobj.c noecho.c \
	beetle.h debug.h noecho.h opcodes.h
libbeetle_la_LIBADD = $(top_builddir)/lib/libgnu.la

bin_PROGRAMS = beetle
beetle_LDADD = libbeetle.la $(top_builddir)/lib/libgnu.la
beetle_SOURCES = beetle.c tbl_opts.h

check-local: libbeetle.la

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_la_SOURCES) $(beetle_SOURCES)

EXTRA_DIST = ARMbeetle.bas
