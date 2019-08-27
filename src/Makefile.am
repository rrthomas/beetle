# Source Makefile.am
#
# (c) Reuben Thomas 2011-2019
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
# RISK.

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbeetle.la
libbeetle_la_SOURCES = storage.c step.c debug.c loadobj.c \
	private.h stringify.h
libbeetle_la_LIBADD = $(top_builddir)/lib/libgnu.la

bin_PROGRAMS = beetle
bin_SCRIPTS = beetle-dump
if HAVE_RLWRAP
bin_SCRIPTS += beetlei
endif
man_MANS = beetle.1 beetlei.1 beetle-dump.1
pkgdata_DATA = completions
beetle_LDADD = libbeetle.la $(top_builddir)/lib/libgnu.la
beetle_SOURCES = main.c tbl_opts.h tbl_commands.h tbl_registers.h $(internal_HDRS)
internal_HDRS = public.h baux.h debug.h opcodes.h
include_HEADERS = beetle.h beetle_aux.h beetle_debug.h beetle_opcodes.h

completions: tbl_registers.h tbl_commands.h
	$(CPP) -I$(srcdir) $(srcdir)/completions.h | grep -v "^#" > $@

debug.lo loadobj.lo step.lo storage.lo main.o: external_syms.h
main.o: tbl_opts.h

# Depend on beetle$(EXEEXT) rather than explicitly make-ing it, as otherwise
# we break parallel builds, as libbeetle.la can be built twice in parallel,
# which can fail. Set distcleancheck_listfiles below to fix distcheck.
beetle.1: beetle$(EXEEXT)
## Exit gracefully if beetle.1 is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --no-info \
		--name="Forth virtual machine" \
		--output=$@ ./beetle$(EXEEXT); \
	fi

do_subst = sed -e 's,[@]pkgdatadir[@],$(pkgdatadir),g' \
	-e 's,[@]docdir[@],$(docdir),g' \
	-e 's,[@]RLWRAP[@],$(RLWRAP),g'

tbl_opts.h: tbl_opts.h.in Makefile
	$(do_subst) < $(srcdir)/tbl_opts.h.in > tbl_opts.h

beetle_opcodes.h: opcodes.h Makefile
	$(do_subst) < $(srcdir)/opcodes.h > $@

beetle.h: public.h external-syms.txt external-defines.txt Makefile
	$(do_subst) < $(srcdir)/public.h > $@.tmp
	cat $(srcdir)/external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $@.tmp > $@
	rm $@.tmp

beetle_aux.h: baux.h external-syms.txt external-defines.txt Makefile
	$(do_subst) < $(srcdir)/baux.h > $@.tmp
	cat $(srcdir)/external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $@.tmp > $@
	rm $@.tmp

beetle_debug.h: debug.h external-syms.txt external-defines.txt Makefile
	$(do_subst) < $(srcdir)/debug.h > $@.tmp
	cat $(srcdir)/external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $@.tmp > $@
	rm $@.tmp

external-defines.txt: $(internal_HDRS)
	$(top_srcdir)/build-aux/find-external-defines $(srcdir) > $@

# Actually needs .o files, but since find-external-syms requires GNU tools,
# don't force remaking external-syms.txt; instead, add it to EXTRA_DIST.
# find-external-syms takes care of remaking the executable.
# N.B. find-external-syms runs make! This can cause duplicate compilation.
$(srcdir)/external-syms.txt: $(libbeetle_la_sources)
	$(top_srcdir)/build-aux/find-external-syms beetle > $@

external_syms.h: external-syms.txt
	sed -e 's/\(.*\)/#define \1 beetle_\1/' < $^ > $@

beetlei: beetlei.in Makefile
	$(do_subst) < $(srcdir)/beetlei.in > $@
	chmod +x $@

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_la_SOURCES) $(beetle_SOURCES)

EXTRA_DIST = ARMbeetle.bas beetlei.in beetlei.1 beetle-dump beetle-dump.1 completions.h tbl_opts.h.in step.c external-syms.txt external_syms.h

DISTCLEANFILES = beetlei beetle.1 completions tbl_opts.h $(include_HEADERS) external-defines.txt external-syms.txt
# Ignore built files that are part of the distribution (specifically,
# beetle.1)
distcleancheck_listfiles = \
       find . -type f -exec sh -c 'test -f $(srcdir)/$$1 || echo $$1' \
	    sh '{}' ';'
