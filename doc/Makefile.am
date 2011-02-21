# Docs Makefile.am

PAPER_SRCS = \
	$(srcdir)/beetle.tex \
	$(srcdir)/cbeetle.tex \
	$(srcdir)/uiface.tex \
	$(srcdir)/intro.tex \
	$(srcdir)/litencode.tex \
	$(srcdir)/tradeoffs.tex

PAPER_PDFS = \
	$(builddir)/beetle.pdf \
	$(builddir)/cbeetle.pdf \
	$(builddir)/uiface.pdf \
	$(builddir)/intro.pdf \
	$(builddir)/litencode.pdf \
	$(builddir)/tradeoffs.pdf

all: pdf

pdf-local: $(PAPER_PDFS)

mostlyclean-local:
	$(LATEXMK) -c $(PAPER_SRCS)

# This would be clean-local, but we distribute the PDFs
maintainer-clean-local:
	$(LATEXMK) -C $(PAPER_SRCS)

EXTRA_DIST = $(PAPER_SRCS) $(PAPER_PDFS)

.tex.pdf:
	$(LATEXMK) -pdf $<
