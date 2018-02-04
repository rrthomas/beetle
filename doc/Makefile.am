# Docs Makefile.am

if GNU_MAKE
BIBINPUTS = $(srcdir)/bibtex/bib
export BIBINPUTS
endif

LATEX_SRCS = \
	$(srcdir)/beetle.tex \
	$(srcdir)/cbeetle.tex \
	$(srcdir)/uiface.tex \
	$(srcdir)/intro.tex \
	$(srcdir)/litencode.tex \
	$(srcdir)/tradeoffs.tex \
	$(srcdir)/bibtex/bib/rrt.bib \
	$(srcdir)/bibtex/bib/vm.bib

LATEX_PDFS = \
	beetle.pdf \
	cbeetle.pdf \
	uiface.pdf \
	intro.pdf \
	litencode.pdf \
	tradeoffs.pdf

all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(RUBBER) --clean $(LATEX_SRCS)

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS)

.tex.pdf:
	$(RUBBER) --pdf $<
