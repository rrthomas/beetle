# Source Makefile.am

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbeetle.la
libbeetle_la_SOURCES = storage.c step.c run.c debug.c loadobj.c \
	beetle_private.h debug.h opcodes.h
libbeetle_la_LIBADD = $(top_builddir)/lib/libgnu.la

bin_PROGRAMS = beetle
if HAVE_RLWRAP
bin_SCRIPTS = beetlei
endif
man_MANS = beetle.1 beetlei.1
pkgdata_DATA = completions
beetle_LDADD = libbeetle.la $(top_builddir)/lib/libgnu.la
beetle_SOURCES = beetle.c tbl_opts.h tbl_commands.h tbl_registers.h
include_HEADERS = beetle.h beetle_aux.h

completions: tbl_registers.h tbl_commands.h
	$(CPP) -I$(srcdir) $(srcdir)/completions.h | grep -v "^#" > $@

beetle.o: tbl_opts.h

beetle.1: beetle.c $(top_srcdir)/configure.ac
## Exit gracefully if beetle.1 is not writeable, such as during distcheck!
	$(MAKE) $(AM_MAKEFLAGS) beetle$(EXEEXT)
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --no-info \
		--name="Forth virtual machine" \
		--output=$@ ./beetle; \
	fi

do_subst = sed -e 's,[@]pkgdatadir[@],$(pkgdatadir),g' \
	-e 's,[@]docdir[@],$(docdir),g' \
	-e 's,[@]RLWRAP[@],$(RLWRAP),g'

tbl_opts.h: tbl_opts.h.in Makefile
	$(do_subst) < $(srcdir)/tbl_opts.h.in > tbl_opts.h

beetlei: beetlei.in Makefile
	$(do_subst) < $(srcdir)/beetlei.in > beetlei
	chmod +x beetlei

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_la_SOURCES) $(beetle_SOURCES) $(include_HEADERS)

EXTRA_DIST = ARMbeetle.bas beetlei.in beetlei.1 completions.h tbl_opts.h.in

DISTCLEANFILES = beetlei beetle.1 completions tbl_opts.h
