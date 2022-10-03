dnl Set default compiler options for use with CYGWIN. Should be called
dnl before AC_PROG_CC and AC_PROG_CXX, but after AC_CYGWIN.

AC_DEFUN([GUP_CYGWIN],
  [
    AC_CYGWIN
    if test x$CYGWIN = xyes; then
      if test "${CFLAGS+set}" = set; then
        CFLAGS="$CFLAGS -mno-cygwin"
      else
        CFLAGS="-g -O2 -mno-cygwin"
      fi
      if test "${CXXFLAGS+set}" = set; then
        CXXFLAGS="$CXXFLAGS -mno-cygwin"
      else
        CXXFLAGS="-g -O2 -mno-cygwin"
      fi
    fi
  ]
)

dnl Set default compiler options for gup. Should be called after AC_PROG_CC
dnl and AC_PROG_CXX.

AC_DEFUN([GUP_COPTIONS],
  [
    if test x$GCC = xyes; then
      CFLAGS="$CFLAGS -Wall -W -Wpointer-arith -Wstrict-prototypes -Wmissing-declarations"
      CXXFLAGS="$CXXFLAGS -Wall -W -Wpointer-arith -Wconversion -Wstrict-prototypes -Wmissing-declarations"
    fi
  ]
)

dnl Check if optimized functions are available for this platform. If
dnl optimized functions are available the test should set enable_index
dnl to the value required by the optimized functions.

AC_DEFUN([GUP_ASSEMBLY],
  [
    if test x$enable_assembly = xyes; then
      asm_subdir=none
      case "$target_cpu" in
      m68*)
        if ${CC-cc} -c $CFLAGS $CPPFLAGS $srcdir/compress/m68000/crc_opti.s -o tmp.o 2>/dev/null; then
          asm_subdir=m68000
          OPTMFILES="opti.c crc_opti.s enc_opti.s sld_opti.s sldfopti.s"
          OPTMINC="opti.h"
          OPTMOBJS="opti.o crc_opti.o enc_opti.o sld_opti.o sldfopti.o"
          enable_index=no
        fi
        if ${CC-cc} -c $CFLAGS $CPPFLAGS $srcdir/compress/m68000.mot/crc_opti.s -o tmp.o 1>/dev/null; then
          asm_subdir=m68000.mot
          OPTMFILES="opti.c crc_opti.s enc_opti.s sld_opti.s sldfopti.s"
          OPTMINC="opti.h"
          OPTMOBJS="opti.o crc_opti.o enc_opti.o sld_opti.o sldfopti.o"
          enable_index=no
        fi
        rm -f tmp.o
        ;;
      esac

      dnl If we have found optimized functions, create necessay links etc.

      if test $asm_subdir != none; then
        echo using optimized functions from compress/$asm_subdir.
        AC_DEFINE(USE_OPTIMIZED)
        if test ! -d compress; then
          mkdir compress
        fi
        (for i in $OPTMFILES; do
          gup_dest="compress/$i"
          rm -f $gup_dest
          case "$srcdir" in
          /*) gup_rel_source="$srcdir/compress/$asm_subdir/$i" ;;
          *) gup_rel_source="../$srcdir/compress/$asm_subdir/$i" ;;
          esac
          if $LN_S $gup_rel_source $gup_dest; then :
          else
            { echo "configure: error: can not link $gup_dest to $gup_rel_source" 1>&2; exit 1; }
          fi
        done)
        if test ! -d guplib; then
          mkdir guplib
        fi
        (for i in $OPTMFILES; do
          gup_dest="guplib/$i"
          rm -f $gup_dest
          case "$srcdir" in
          /*) gup_rel_source="$srcdir/compress/$asm_subdir/$i" ;;
          *) gup_rel_source="../$srcdir/compress/$asm_subdir/$i" ;;
          esac
          if $LN_S $gup_rel_source $gup_dest; then :
          else
            { echo "configure: error: can not link $gup_dest to $gup_rel_source" 1>&2; exit 1; }
          fi
        done)
        if test ! -d include; then
          mkdir include
        fi
        (for i in $OPTMINC; do
          gup_dest="include/$i"
          rm -f $gup_dest
          case "$srcdir" in
          /*) gup_rel_source="$srcdir/compress/$asm_subdir/$i" ;;
          *) gup_rel_source="../$srcdir/compress/$asm_subdir/$i" ;;
          esac
          if $LN_S $gup_rel_source $gup_dest; then :
          else
            { echo "configure: error: can not link $gup_dest to $gup_rel_source" 1>&2; exit 1; }
          fi
        done)
      fi
    fi

    OPTMLTOBJS="`echo ${OPTMOBJS} | sed 's/\.o/.lo/g'`"

    AC_SUBST(OPTMFILES)
    AC_SUBST(OPTMINC)
    AC_SUBST(OPTMOBJS)
    AC_SUBST(OPTMLTOBJS)
  ]
)

dnl Check if the compression engine should use pointers or indices for
dnl optimal performance. This test should be after GUP_ASSEMBLY,
dnl because GUP_ASSEMBLY changes enable_index if optimized functions
dnl are found.

AC_DEFUN([GUP_USE_INDEX],
  [
    if test x$enable_index = xcheck; then
      case "$target_cpu" in
      m68*)
        enable_index=no;
        ;;
      *)
        enable_index=yes;
        ;;
      esac
    fi
    if test x$enable_index = xyes; then
      AC_DEFINE(INDEX_STRUCT)
      echo using indices in the compression engine.
    else
      echo using pointers in the compression engine.
    fi
  ]
)

dnl Check the OS id that gup should use. Currently the OS id is
dnl always OS_UNIX.

AC_DEFUN([GUP_OS_ID],
  [
    if test x$CYGWIN = xyes; then
      os_id=OS_WIN32
      AMOSDEPOBJS="w32_arj.o w32_gz.o w32_lha.o w32_utl.o"
      LIBOBJS="$LIBOBJS w32_err.o"
    else
      os_id=OS_UNIX
      AMOSDEPOBJS="unix_arj.o unix_gz.o unix_lha.o unix_utl.o"
    fi
    AC_DEFINE_UNQUOTED(OS, $os_id)
    echo using OS id $os_id.

    AMOSDEPLTOBJS="`echo ${AMOSDEPOBJS} | sed 's/\.o/.lo/g'`"

    AC_SUBST(AMOSDEPOBJS)
    AC_SUBST(AMOSDEPLTOBJS)
  ]
)

dnl Check for ints.h.

AC_DEFUN([GUP_INTS_H],
  [
    AC_CHECK_HEADER(ints.h, AC_DEFINE(HAVE_INTS_H))
  ]
)

dnl Check if a given type exists. If true, define HAVE_X,
dnl where X is the type in uppercase. This function is used
dnl to test the presence of the int8, uint8 etc. types and
dnl therefore depends on ints.h.

AC_DEFUN([GUP_CHECK_TYPE],
  [
    AC_REQUIRE([GUP_INTS_H])dnl
    AC_MSG_CHECKING(for $1)
    AC_CACHE_VAL(ac_cv_type_$1,
      [AC_EGREP_CPP(dnl
      changequote(<<,>>)dnl
      <<$1[^a-zA-Z_0-9]>>dnl
      changequote([,]), [#include <sys/types.h>
      #if HAVE_INTS_H
      #include <ints.h>
      #endif], ac_cv_type_$1=yes, ac_cv_type_$1=no)])dnl
    AC_MSG_RESULT($ac_cv_type_$1)
    if test $ac_cv_type_$1 = yes; then
        AC_DEFINE_UNQUOTED($2)
    fi
  ]
)

dnl Check if the compiler supports the long long type.

AC_DEFUN([GUP_CHECK_LONG_LONG],
  [
    AC_MSG_CHECKING(for long long type)
    AC_CACHE_VAL(ac_cv_type_long_long,
      [
        AC_TRY_LINK(
          [
          ],
          [
            int main(void)
            {
              long long a = 1, b = 2;

              return (int)(a + b);
            }
          ], ac_cv_type_long_long=yes, ac_cv_type_long_long=no)
      ])dnl
    AC_MSG_RESULT($ac_cv_type_long_long)
    if test $ac_cv_type_long_long = yes; then
        AC_DEFINE(HAVE_LONG_LONG)
    fi
  ]
)
