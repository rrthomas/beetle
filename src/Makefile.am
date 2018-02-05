# Source Makefile.am

SUBDIRS = . tests

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib

# FIXME: Install the library, for use by other programs
noinst_LIBRARIES = libbeetle.a
libbeetle_a_SOURCES = storage.c step.c run.c debug.c loadobj.c saveobj.c noecho.c \
	beetle.h debug.h noecho.h opcodes.h

bin_PROGRAMS = beetle
beetle_LDADD = libbeetle.a $(top_builddir)/lib/libgnu.a
beetle_SOURCES = beetle.c tbl_opts.h

check-local: libbeetle.a

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_a_SOURCES) $(beetle_SOURCES)

EXTRA_DIST = ARMbeetle,ffb
