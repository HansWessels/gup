dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])
AC_INIT([gup],[0.0.7])
AC_CONFIG_SRCDIR([compress/crc.c])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(include/config.h)
AC_CONFIG_MACRO_DIRS([m4])

AC_ARG_ENABLE(assembly, [  --enable-assembly       enable optimized assembly functions], , enable_assembly=yes)
AC_ARG_ENABLE(index, [  --enable-index          enable use of indices in the compression engine], , enable_index=check)
AM_MAINTAINER_MODE
GUP_CYGWIN

dnl define templates for config.h.
dnl AC_DEFINE([USE_OPTIMIZED], [], [Define if optimized assembly functions are available.])
dnl AC_DEFINE([INDEX_STRUCT], [], [Define if the compression engine should use indices instead of pointers.])
dnl AC_DEFINE([OS], [], [OS id used by gup.])
dnl AC_DEFINE([SYSTEM], [], [System gup is build for.])
dnl AC_DEFINE([HAVE_INTS_H], [], [Define if ints.h exists.])
dnl AC_DEFINE([HAVE_LONG_LONG], [], [Define if the the long long type exists.])
dnl AC_DEFINE([HAVE_INT8], [], [Define if the the int8 type exists.])
dnl AC_DEFINE([HAVE_INT16], [], [Define if the the int16 type exists.])
dnl AC_DEFINE([HAVE_INT32], [], [Define if the the int32 type exists.])
dnl AC_DEFINE([HAVE_INT64], [], [Define if the the int64 type exists.])
dnl AC_DEFINE([HAVE_UINT8], [], [Define if the the uint8 type exists.])
dnl AC_DEFINE([HAVE_UINT16], [], [Define if the the uint16 type exists.])
dnl AC_DEFINE([HAVE_UINT32], [], [Define if the the uint32 type exists.])
dnl AC_DEFINE([HAVE_UINT64], [], [Define if the the uint64 type exists.])
dnl AC_DEFINE([WITH_DMALLOC], [], [dmalloc library.])
dnl AC_DEFINE([__EXTENSIONS__], [], [__EXTENSIONS__.])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
GUP_COPTIONS
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(AR, ar, :)
AC_CHECK_TOOL(NM, nm, :)
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_DISABLE_STATIC([])
LT_INIT

dnl Checks for libraries.
AM_WITH_DMALLOC

dnl Checks for header files.
m4_warn([obsolete],
[The preprocessor macro `STDC_HEADERS' is obsolete.
  Except in unusual embedded environments, you can safely include all
  ISO C90 headers unconditionally.])dnl
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

AC_CHECK_HEADERS(unistd.h rx.h regex.h io.h)
GUP_INTS_H

dnl Checks for typedefs, structures, and compiler characteristics.
GUP_CHECK_LONG_LONG
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
if test $ac_cv_type_long_long = yes; then
  AC_CHECK_SIZEOF(long long, 8)
fi
GUP_CHECK_TYPE(int8, HAVE_INT8)
GUP_CHECK_TYPE(uint8, HAVE_UINT8)
GUP_CHECK_TYPE(int16, HAVE_INT16)
GUP_CHECK_TYPE(uint16, HAVE_UINT16)
GUP_CHECK_TYPE(int32, HAVE_INT32)
GUP_CHECK_TYPE(uint32, HAVE_UINT32)
GUP_CHECK_TYPE(int64, HAVE_INT64)
GUP_CHECK_TYPE(uint64, HAVE_UINT64)
GUP_POSIX_199506_DEFINE

dnl Checks for library functions.
AC_FUNC_MMAP
AC_REPLACE_FUNCS(strlwr)
AC_CHECK_FUNCS(getpwnam_r getpwuid_r getgrnam_r getgrgid_r localtime_r lchown)

dnl Define system type.
AC_DEFINE_UNQUOTED(SYSTEM, "$target_alias")

dnl GUP specific tests. First check if optimized assembly functions are
dnl available. Next check if the compression engine should use indices
dnl or pointers.
GUP_OS_ID
GUP_ASSEMBLY
GUP_USE_INDEX

dnl The following two lines are needed for autoconf 2.12 and earlier.
LTLIBOBJS="`echo ${LIBOBJS} | sed 's/\.o/.lo/g'`"
AC_SUBST(LTLIBOBJS)

AC_CONFIG_FILES([Makefile lib/Makefile compress/Makefile])
AC_OUTPUT
AC_CONFIG_FILES([include/Makefile include/sysdep/Makefile arcman/Makefile guplib/Makefile unix/Makefile])
AC_OUTPUT
AC_CONFIG_FILES([gup.spec])
AC_OUTPUT
