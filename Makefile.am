SUBDIRS = src

MAINTAINERCLEANFILES = \
        Makefile.in src/Makefile.in config.h.in configure \
        install-sh ltmain.sh missing mkinstalldirs \
        config.log config.status config.guess config.sub config.h \
        build-stamp compile depcomp acinclude.m4 aclocal.m4 \
	stamp-h1 

EXTRA_DIST = autogen.sh

dist-hook:
	cp -va $(top_srcdir)/src/*.h $(top_distdir)/src
