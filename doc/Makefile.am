# Docs Makefile.am
#
# (c) Reuben Thomas 2011-2018
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

LATEX_SRCS = \
	$(srcdir)/papers/intro.tex \
	$(srcdir)/papers/litencode.tex \
	$(srcdir)/papers/tradeoffs.tex \
	$(srcdir)/bibtex/bib/rrt.bib \
	$(srcdir)/bibtex/bib/vm.bib

LATEX_PDFS = \
	beetle.pdf \
	cbeetle.pdf \
	shell.pdf \
	papers/intro.pdf \
	papers/litencode.pdf \
	papers/tradeoffs.pdf

if HAVE_LATEXMK
dist_doc_DATA = $(LATEX_PDFS)
endif


all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(LATEXMK) -r $(srcdir)/latexmkrc -C -f $(LATEX_SRCS) \
		$(builddir)/beetle.tex $(builddir)/cbeetle.tex $(builddir)/shell.tex

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS) cbeetle.tex.in shell.tex.in latexmkrc

# TODO: mv works around a bug in latexmk < 4.54, which sets $out_dir when it
# shouldn't. Remove when we can require latexmk >= 4.54 (Ubuntu 20.04).
.tex.pdf:
	env BIBINPUTS=$(abs_srcdir)/bibtex/bib $(LATEXMK) -r $(abs_srcdir)/latexmkrc -cd -f $< && \
	if test -e `basename $@` -a ! `basename $@` -ef $@; then \
	    mv `basename $@` $@; \
	fi
