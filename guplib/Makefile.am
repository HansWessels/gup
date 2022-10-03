lib_LTLIBRARIES = libgup.la

libgup_la_SOURCES = arc_util.cpp archive.cpp arj_arc.cpp arj_hdr.cpp \
					crc.c decode.c encode.c evaluatr.c gup_err.c gup_io.c \
					gz_arc.cpp gz_hdr.cpp header.cpp lha_arc.cpp \
					lha_hdr.cpp matchpat.c sld.c sld_indx.c sld_med.c \
					sld_smll.c

EXTRA_libgup_la_SOURCES = unix_arj.cpp unix_gz.cpp unix_lha.cpp \
                          unix_utl.cpp w32_arj.cpp w32_gz.cpp w32_lha.cpp \
						  w32_utl.cpp

libgup_la_LIBADD = @LTLIBOBJS@ @OPTMLTOBJS@ @AMOSDEPLTOBJS@
libgup_la_DEPENDENCIES = $(libgup_la_LIBADD)
libgup_la_LDFLAGS = -version-info 0:0:0

noinst_HEADERS = arc_util.h arcctl.h archive.h arj_arc.h arj_hdr.h decode.h \
				 encode.h evaluatr.h header.h lha_arc.h lha_hdr.h os.h

AM_CPPFLAGS = -I$(top_srcdir)/include

DISTCLEANFILES = @OPTMFILES@
