# Makefile for C Beetle
# Reuben Thomas   11/94-17/2/11

AM_CPPFLAGS = -I$(top_srcdir)
#LDADD = $(top_builddir)/lib/libgnu.a

bin_PROGRAMS = beetle

#beetle_LDADD = $(LDADD) $(LIBOBJS)

beetle_base_SOURCES = storage.c tests.c step.c run.c lib.c debug.c loadobj.c saveobj.c noecho.c

beetle_SOURCES = $(beetle_base_SOURCES) beetle.c

check-local:
	$(MAKE) -C tests check
