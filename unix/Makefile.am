bin_PROGRAMS = gup

gup_SOURCES = main.cpp decomp.cpp compress.cpp utils.cpp scan.cpp
gup_LDADD = ../arcman/libarcman.a ../compress/libcompress.a ../lib/libsupport.a
gup_DEPENDENCIES = $(gup_LDADD)

noinst_HEADERS = arj.h scan.h utils.h

AM_CPPFLAGS = -I$(top_srcdir)/include
