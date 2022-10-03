noinst_LIBRARIES = libsupport.a

libsupport_a_SOURCES = gup_err.c gup_io.c matchpat.c
libsupport_a_LIBADD = @LIBOBJS@
libsupport_a_DEPENDENCIES = $(libsupport_a_LIBADD)

AM_CPPFLAGS = -I./include -I$(top_srcdir)/include
