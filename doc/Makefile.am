# Docs Makefile.am
#
# (c) Reuben Thomas 2011-2025
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

LATEX_SRCS = \
	$(srcdir)/beetle.tex \
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

dist_doc_DATA = $(LATEX_PDFS)

all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(LATEXMK) -r $(srcdir)/latexmkrc -C -f $(LATEX_SRCS) \
		$(builddir)/beetle.tex $(builddir)/cbeetle.tex $(builddir)/shell.tex

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS) cbeetle.tex.in shell.tex.in latexmkrc
