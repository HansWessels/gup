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

gup:
	cd unix
	make

test: gup
	echo "=== show on-line help output ==="
	$(GUP_EXE) 
	echo "=== show EXTENDED on-line help output ==="
	$(GUP_EXE) -h
	echo "=== compressing Calgary Corpus ==="
	$(GUP_EXE) a -m7 -jm -e test.arj test/calgary-corpus/[a-z]*
	echo "=== testing Calgary Corpus ARJ archive ==="
	$(GUP_EXE) t test.arj
	./test_check_archive_size.sh  test.arj
	make testdump

testdump: testcdump testasmdump testbindump

testcdump: gup
	-rm test.cdump*
	echo "=== testing DUMP MODES: C ==="
	$(GUP_EXE) a test.cdump test/test-data/src/ 
	cat test.cdump | sed -E -e 's/creation time:\s+[0-9]+/creation time:      <TIMESTAMP>/' | tee test.cdump.hexdump
	if ! test -e test.sollwert.cdump.hexdump.txt ; then cp test.cdump.hexdump test.sollwert.cdump.hexdump.txt ; fi
	diff -u test.cdump.hexdump test.sollwert.cdump.hexdump.txt

testasmdump: gup
	-rm test.asmdump*
	echo "=== testing DUMP MODES: ASM ==="
	$(GUP_EXE) a test.asmdump test/test-data/src/
	cat test.asmdump | sed -E -e 's/creation time:\s+[0-9]+/creation time:      <TIMESTAMP>/' | tee test.asmdump.hexdump
	if ! test -e test.sollwert.asmdump.hexdump.txt ; then cp test.asmdump.hexdump test.sollwert.asmdump.hexdump.txt ; fi
	diff -u test.asmdump.hexdump test.sollwert.asmdump.hexdump.txt

testbindump: gup
	-rm test.bindump*
	echo "=== testing DUMP MODES: RAW BINARY ==="
	$(GUP_EXE) a test.bindump test/test-data/src/
	od -A d -t x1 test.bindump | tee test.bindump.hexdump
	if ! test -e test.sollwert.bindump.hexdump.txt ; then cp test.bindump.hexdump test.sollwert.bindump.hexdump.txt ; fi
	diff -u test.bindump.hexdump test.sollwert.bindump.hexdump.txt

clean:
	-rm **/*.bak
	-rm **/*.i
	-rm **/*~
	-rm **/*.o **/*.a **/*.lo **/*.la **/*.obj
	-rm **/*.meta.nfo
	-rm **/*.cdump **/*.asmdump **/*.bindump
	-rm $(GUP_EXE)

# only invoke this make target when you want to update/regenerate the test reference values & files:
clean_test_sollwerte:
	-rm -f test.sollwert.cdump.hexdump.txt
	-rm -f test.sollwert.asmdump.hexdump.txt 
	-rm -f test.sollwert.bindump.hexdump.txt

superclean: clean
	make distclean

.PHONY: gup test dist-hook distall clean distclean superclean testdump testcdump testasmdump testbindump clean_test_sollwerte
