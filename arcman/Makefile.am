noinst_LIBRARIES = libarcman.a

libarcman_a_SOURCES = arc_util.cpp archive.cpp arj_arc.cpp arj_hdr.cpp \
					  gz_arc.cpp gz_hdr.cpp header.cpp lha_arc.cpp lha_hdr.cpp \
					  dump_arc.cpp dump_hdr.cpp

EXTRA_libarcman_a_SOURCES = unix_arj.cpp unix_gz.cpp unix_lha.cpp \
							unix_utl.cpp w32_arj.cpp w32_gz.cpp \
							w32_lha.cpp w32_utl.cpp

libarcman_a_LIBADD = @AMOSDEPOBJS@
libarcman_a_DEPENDENCIES = $(libarcman_a_LIBADD)

noinst_HEADERS = arc_util.h arcctl.h archive.h arj_arc.h arj_hdr.h \
				 gz_arc.h gz_hdr.h header.h \
				 lha_arc.h lha_hdr.h os.h #
				 dump_arc.h dump_hdr.h 

AM_CPPFLAGS = -I$(top_srcdir)/include

dist-hook:
	mkdir -p $(distdir)/include/unix
	mkdir -p $(distdir)/include/win32
	cp $(srcdir)/include/unix/*.h $(distdir)/include/unix
	cp $(srcdir)/include/win32/*.h $(distdir)/include/win32
