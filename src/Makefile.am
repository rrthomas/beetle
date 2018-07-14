# Source Makefile.am
#
# (c) Reuben Thomas 2011-2018
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
# RISK.

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbeetle.la
libbeetle_la_SOURCES = storage.c step.c run.c debug.c loadobj.c \
	private.h stringify.h
libbeetle_la_LIBADD = $(top_builddir)/lib/libgnu.la

bin_PROGRAMS = beetle
if HAVE_RLWRAP
bin_SCRIPTS = @PACKAGE@i
endif
man_MANS = @PACKAGE@.1 @PACKAGE@i.1
pkgdata_DATA = completions
beetle_LDADD = lib@PACKAGE@.la $(top_builddir)/lib/libgnu.la
beetle_SOURCES = main.c tbl_opts.h tbl_commands.h tbl_registers.h $(internal_HDRS)
internal_HDRS = public.h aux.h debug.h opcodes.h
include_HEADERS = @PACKAGE@.h @PACKAGE@_aux.h @PACKAGE@_debug.h @PACKAGE@_opcodes.h

completions: tbl_registers.h tbl_commands.h
	$(CPP) -I$(srcdir) $(srcdir)/completions.h | grep -v "^#" > $@

debug.lo loadobj.lo run.lo step.lo storage.lo main.o: external_syms.h
main.o: tbl_opts.h

# Depend on @PACKAGE@$(EXEEXT) rather than explicitly make-ing it, as otherwise
# we break parallel builds, as lib@PACKAGE@.la can be built twice in parallel,
# which can fail. Set distcleancheck_listfiles below to fix distcheck.
@PACKAGE@.1: @PACKAGE@$(EXEEXT)
## Exit gracefully if @PACKAGE@.1 is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --no-info \
		--name="Forth virtual machine" \
		--output=$@ ./@PACKAGE@$(EXEEXT); \
	fi

do_subst = sed -e 's,[@]pkgdatadir[@],$(pkgdatadir),g' \
	-e 's,[@]docdir[@],$(docdir),g' \
	-e 's,[@]PACKAGE[@],$(PACKAGE),g' \
	-e 's,[@]PACKAGE_NAME[@],$(PACKAGE_NAME),g' \
	-e 's,[@]RLWRAP[@],$(RLWRAP),g'

tbl_opts.h: tbl_opts.h.in Makefile
	$(do_subst) < $(srcdir)/tbl_opts.h.in > tbl_opts.h

@PACKAGE@_opcodes.h: opcodes.h
	cp $< $@

@PACKAGE@.h: public.h external-syms.txt external-defines.txt
	cat external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $< $@

@PACKAGE@_aux.h: aux.h external-syms.txt external-defines.txt
	cat external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $< $@

@PACKAGE@_debug.h: debug.h external-syms.txt external-defines.txt
	cat external-syms.txt external-defines.txt | $(top_srcdir)/build-aux/add-symbol-prefix $< $@

external-defines.txt: $(internal_HDRS)
	$(top_builddir)/build-aux/find-external-defines > $@

# Actually needs .o files, but since find-external-syms requires GNU tools,
# don't force remaking external-syms.txt; instead, add it to EXTRA_DIST.
# find-external-syms takes care of remaking the executable.
# (Dependency is libbeetle_la_SOURCES, but with .h.in's instead of .h's)
$(srcdir)/external-syms.txt: storage.c step.c.in run.c debug.c loadobj.c private.h.in stringify.h
	$(top_srcdir)/build-aux/find-external-syms $(PACKAGE) > $@

external_syms.h: external-syms.txt
	sed -e 's/\(.*\)/#define \1 $(PACKAGE)_\1/' < $^ > $@

@PACKAGE@i: @PACKAGE@i.in Makefile
	$(do_subst) < $(srcdir)/@PACKAGE@i.in > @PACKAGE@i
	chmod +x @PACKAGE@i

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(lib@PACKAGE@_la_SOURCES) $(@PACKAGE@_SOURCES) $(include_HEADERS)

EXTRA_DIST = ARMbeetle.bas @PACKAGE@i.in @PACKAGE@i.1 completions.h tbl_opts.h.in step.c.in external-syms.txt external_syms.h

DISTCLEANFILES = @PACKAGE@i @PACKAGE@.1 @PACKAGE@i.1 completions tbl_opts.h $(include_HEADERS) external-defines.txt external-syms.txt
# Ignore built files that are part of the distribution (specifically,
# @PACKAGE@.1)
distcleancheck_listfiles = \
       find . -type f -exec sh -c 'test -f $(srcdir)/$$1 || echo $$1' \
	    sh '{}' ';'
