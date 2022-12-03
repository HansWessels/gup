noinst_LIBRARIES = libcompress.a

libcompress_a_SOURCES = crc.c decode.c encode.c evaluatr.c sld.c sld_indx.c sld_med.c sld_smll.c sld32.c n0.c n1.c n2.c n9.c arj_m4.c
libcompress_a_LIBADD = @OPTMOBJS@
libcompress_a_DEPENDENCIES = $(libcompress_a_LIBADD)

noinst_HEADERS = decode.h encode.h evaluatr.h

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = compress.prj
DISTCLEANFILES = @OPTMFILES@

dist-hook:
	(for i in st_opti m68000 m68000.mot; do mkdir $(distdir)/$$i; \
	 cp -p $(srcdir)/$$i/*.[chs] $(distdir)/$$i; done)
	cp -p $(srcdir)/st_opti/command.mac $(distdir)/st_opti
