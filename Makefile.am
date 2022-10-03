AUTOMAKE_OPTIONS = 1.4 dist-zip

ACLOCAL_AMFLAGS = '-I m4'

SUBDIRS = include lib compress arcman guplib unix

EXTRA_DIST = acconfig.h

GUP_EXE = ./unix/gup

RPMROOT = /usr/src/redhat

dist-arj dist-gup: distdir
	-chmod -R a+r $(distdir)
	mv $(distdir) $(PACKAGE)
	$(GUP_EXE) a -m1 $(distdir).arj $(PACKAGE)/*
	rm -rf $(PACKAGE)

dist-bzip2: distdir
	-chmod -R a+r $(distdir)
	$(TAR) --use-compress-program=bzip2 -chof $(distdir).tar.bz2 $(distdir)/*
	rm -rf $(distdir)

distall: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	$(TAR) --use-compress-program=bzip2 -chof $(distdir).tar.bz2 $(distdir)/*
	zip -rq $(distdir).zip $(distdir)
	mv $(distdir) $(PACKAGE)
	$(GUP_EXE) a -m1 $(distdir).arj $(PACKAGE)/*
	rm -rf $(PACKAGE)

dist-hook:
	mkdir $(distdir)/arjbeta
	cp -p $(srcdir)/arjbeta/*.[chs] $(srcdir)/arjbeta/arjbeta.prj $(distdir)/arjbeta
	mkdir -p $(distdir)/win32/bcb5
	cp -p $(srcdir)/win32/bcb5/Gup.* $(srcdir)/win32/bcb5/README.txt $(distdir)/win32/bcb5

$(PACKAGE)-$(VERSION).tar.gz:
	make dist

rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(distdir).tar.gz $(RPMROOT)/SOURCES
	rpm -ba gup.spec
	rm $(RPMROOT)/SOURCES/$(distdir).tar.gz

test:
	pwd
	$(GUP_EXE) -h

