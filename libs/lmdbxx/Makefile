# Makefile for lmdb++ <http://lmdbxx.sourceforge.net/>

PACKAGE_NAME      := lmdb++
PACKAGE_TARNAME   := lmdbxx
PACKAGE_VERSION    = $(shell cat VERSION)
PACKAGE_STRING     = $(PACKAGE_NAME) $(PACKAGE_TARNAME)
PACKAGE_TARSTRING  = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)
PACKAGE_BUGREPORT := arto@bendiken.net
PACKAGE_URL       := http://lmdbxx.sourceforge.net/

DESTDIR  :=
PREFIX   := /usr/local

CPPFLAGS := -I.
CXXFLAGS := -g -O0 -std=c++11 -Wall -Werror
LDFLAGS  :=
LDADD    := -llmdb

includedir = $(PREFIX)/include

MKDIR         := mkdir -p
RM            := rm -f
INSTALL       := install -c
INSTALL_DATA   = $(INSTALL) -m 644
INSTALL_HEADER = $(INSTALL_DATA)

DISTFILES := AUTHORS CREDITS INSTALL README TODO UNLICENSE VERSION \
             Makefile check.cc example.cc lmdb++.h

default: help

help:
	@echo 'Install the <lmdb++.h> header file using `make install`.'

check: check.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDADD) && ./$@

example: example.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDADD) && ./$@

%.o: %.cc lmdb++.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

installdirs:
	$(MKDIR) $(DESTDIR)$(includedir)

install: lmdb++.h installdirs
	$(INSTALL_HEADER) $< $(DESTDIR)$(includedir)

uninstall:
	$(RM) $(DESTDIR)$(includedir)/lmdb++.h

clean:
	$(RM) README.html README.md check example $(PACKAGE_TARSTRING).tar.* *.o *~

README: README.html README.md

README.html: README.rst
	pandoc -s -f rst -t html5 -S -o $@ $<

README.md: README.rst
	pandoc -s -f rst -t markdown_github -o - $< | tail -n +5 > $@

doxygen: README.md
	doxygen Doxyfile
	sed -e 's/Main Page/a C++11 wrapper for LMDB/'                    \
	    -e 's/lmdb++ Documentation/lmdb++: a C++11 wrapper for LMDB/' \
	    -i.orig .doxygen/html/index.html

maintainer-clean: clean

maintainer-doxygen: doxygen
	rsync -az .doxygen/html/ bendiken@web.sourceforge.net:/home/project-web/lmdbxx/htdocs/

dist:
	tar -chJf $(PACKAGE_TARSTRING).tar.xz \
	    --transform 's,^,$(PACKAGE_TARSTRING)/,' $(DISTFILES)
	tar -chjf $(PACKAGE_TARSTRING).tar.bz2 \
	    --transform 's,^,$(PACKAGE_TARSTRING)/,' $(DISTFILES)
	tar -chzf $(PACKAGE_TARSTRING).tar.gz \
	    --transform 's,^,$(PACKAGE_TARSTRING)/,' $(DISTFILES)

.PHONY: help check example installdirs install uninstall clean doxygen maintainer-doxygen dist
