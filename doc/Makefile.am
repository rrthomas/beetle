# Docs Makefile.am

if GNU_MAKE
BIBINPUTS = $(abs_srcdir)/bibtex/bib
export BIBINPUTS
endif

LATEX_SRCS = \
	$(srcdir)/beetle.tex \
	$(srcdir)/cbeetle.tex \
	$(srcdir)/uiface.tex \
	$(srcdir)/papers/intro.tex \
	$(srcdir)/papers/litencode.tex \
	$(srcdir)/papers/tradeoffs.tex \
	$(srcdir)/bibtex/bib/rrt.bib \
	$(srcdir)/bibtex/bib/vm.bib

LATEX_PDFS = \
	beetle.pdf \
	cbeetle.pdf \
	uiface.pdf \
	papers/intro.pdf \
	papers/litencode.pdf \
	papers/tradeoffs.pdf

all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(RUBBER) --clean $(LATEX_SRCS)

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS)

.tex.pdf:
	$(RUBBER) --pdf --inplace $<
