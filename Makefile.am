# Top-level Makefile.am

SUBDIRS = lib src doc

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = m4/gnulib-cache.m4 README.md

release: distcheck
	git diff --exit-code && \
	git tag -a -m "Release tag" "v$(VERSION)" && \
	git push && git push --tags && \
	woger github \
		github_user=rrthomas \
		package=$(PACKAGE) \
		version=$(VERSION) \
		dist_type=tar.gz
