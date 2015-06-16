# Docs Makefile.am

LATEX_SRCS = \
	$(srcdir)/beetle.tex \
	$(srcdir)/cbeetle.tex \
	$(srcdir)/uiface.tex \
	$(srcdir)/intro.tex \
	$(srcdir)/litencode.tex \
	$(srcdir)/tradeoffs.tex

LATEX_PDFS = \
	$(builddir)/beetle.pdf \
	$(builddir)/cbeetle.pdf \
	$(builddir)/uiface.pdf \
	$(builddir)/intro.pdf \
	$(builddir)/litencode.pdf \
	$(builddir)/tradeoffs.pdf

all: pdf

pdf-local: $(LATEX_PDFS)

MAINTAINERCLEANFILES = $(LATEX_PDFS)

clean-local:
	$(RUBBER) --clean $(LATEX_SRCS)

EXTRA_DIST = $(LATEX_SRCS) $(LATEX_PDFS)

.tex.pdf:
	$(RUBBER) --pdf $<
