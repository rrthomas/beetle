# Source Makefile.am
#
# (c) Reuben Thomas 2011-2019
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbeetle.la
libbeetle_la_SOURCES = vm.c debug.c loadobj.c private.h stringify.h
libbeetle_la_LIBADD = $(top_builddir)/lib/libgnu.la
if HAVE_MIJIT
libbeetle_la_LIBADD += $(top_srcdir)/mijit-beetle/target/release/libmijit_beetle.la
endif

bin_PROGRAMS = beetle
bin_SCRIPTS = beetle-dump
if HAVE_RLWRAP
bin_SCRIPTS += beetlei
endif
man_MANS = beetle.1 beetlei.1 beetle-dump.1
pkgdata_DATA = completions
beetle_LDADD = libbeetle.la $(top_builddir)/lib/libgnu.la
beetle_SOURCES = main.c tbl_opts.h tbl_commands.h tbl_registers.h $(include_HEADERS)
include_HEADERS = beetle.h beetle_aux.h beetle_debug.h beetle_opcodes.h

completions: tbl_registers.h tbl_commands.h
	$(CPP) -I$(srcdir) $(srcdir)/completions.h | grep -v "^#" > $@

main.o: tbl_opts.h

if HAVE_MIJIT
.PHONY: mijit-beetle
$(top_srcdir)/mijit-beetle/target/release/libmijit_beetle.la: mijit-beetle
	cargo build --release --manifest-path $(top_srcdir)/mijit-beetle/Cargo.toml
endif

# Depend on beetle$(EXEEXT) rather than explicitly make-ing it, as otherwise
# we break parallel builds, as libbeetle.la can be built twice in parallel,
# which can fail.
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

beetlei: beetlei.in Makefile
	$(do_subst) < $(srcdir)/beetlei.in > $@
	chmod +x $@

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbeetle_la_SOURCES) $(beetle_SOURCES)

EXTRA_DIST = ARMbeetle.bas beetlei.in beetlei.1 beetle-dump beetle-dump.1 completions.h tbl_opts.h.in vm.c external_syms.h

DISTCLEANFILES = beetlei beetle.1 completions tbl_opts.h $(include_HEADERS)
