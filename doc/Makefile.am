# Docs Makefile.am

if GNU_MAKE
BIBINPUTS = $(abs_srcdir)/bibtex/bib
export BIBINPUTS
endif

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
	debugger.pdf \
	papers/intro.pdf \
	papers/litencode.pdf \
	papers/tradeoffs.pdf

dist_doc_DATA = $(LATEX_PDFS)


all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(LATEXMK) -r $(srcdir)/latexmkrc -C -f $(LATEX_SRCS) $(builddir)/cbeetle.tex $(builddir)/debugger.tex

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS) cbeetle.tex.in debugger.tex.in latexmkrc

.tex.pdf:
	$(LATEXMK) -r $(abs_srcdir)/latexmkrc -cd -f $<
