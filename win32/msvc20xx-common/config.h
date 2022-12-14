#ifndef _MSC_VER
#error "SHOULD NOT include this header file when you're not using Microsoft Visual Studio 2017/2019/2022/..."
#endif

/* include/config.h.  Generated from config.h.in by configure.  */
/* include/config.h.in.  Generated from configure.ac by autoheader.  */
#define PACKAGE "gup"
#define VERSION "0.0.7"

/* Define if optimized assembly functions are available. */
/* #undef USE_OPTIMIZED */

/* Define if the compression engine should use indices instead of pointers. */
#define INDEX_STRUCT 1

/* OS id used by gup. */
#define OS OS_WIN32

/* System gup is build for. */
#define SYSTEM ""

/* Define if ints.h exists. */
/* #undef HAVE_INTS_H */

/* Define if the the long long type exists. */
#define HAVE_LONG_LONG 1

/* Define if the the int8 type exists. */
#define HAVE_INT8 1

/* Define if the the uint8 type exists. */
#define HAVE_UINT8 1

/* Define if the the int16 type exists. */
#define HAVE_INT16 1

/* Define if the the uint16 type exists. */
#define HAVE_UINT16 1

/* Define if the the int32 type exists. */
#define HAVE_INT32 1

/* Define if the the uint32 type exists. */
#define HAVE_UINT32 1

/* Define if the the int64 type exists. */
#define HAVE_INT64 1

/* Define if the the uint64 type exists. */
#define HAVE_UINT64 1

#include <stdint.h>

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;


/* dmalloc library. */
/* #undef WITH_DMALLOC */

/* _POSIX_C_SOURCE. */
/* #undef _POSIX_C_SOURCE */

/* __EXTENSIONS__. */
/* #undef __EXTENSIONS__ */

/* Define to 1 if you have the <direct.h> header file. */
#define HAVE_DIRECT_H 1

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dir.h> header file. */
// #undef HAVE_DIR_H 

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `getgrgid_r' function. */
/* #undef HAVE_GETGRGID_R */

/* Define to 1 if you have the `getgrnam_r' function. */
/* #undef HAVE_GETGRNAM_R */

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getpwnam_r' function. */
/* #undef HAVE_GETPWNAM_R */

/* Define to 1 if you have the `getpwuid_r' function. */
/* #undef HAVE_GETPWUID_R */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#define HAVE_IO_H 1

/* Define to 1 if you have the `lchown' function. */
/* #undef HAVE_LCHOWN */

/* Define to 1 if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have a working `mmap' system call. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <regex.h> header file. */
/* #undef HAVE_REGEX_H */

/* Define to 1 if you have the <rx.h> header file. */
/* #undef HAVE_RX_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlwr' function. */
#define HAVE_STRLWR 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <tos.h> header file. */
/* #undef HAVE_TOS_H */

/* Define to 1 if you have the <unistd.h> header file. */
//#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <utime.h> header file. */
// #undef HAVE_UTIME_H 

/* Define to 1 if you have the `_mkdir' function. */
#define HAVE__MKDIR 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "gup"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "gup"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "gup 0.0.7"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gup"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.7"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#if defined(_WIN64) || defined(WIN64)
#define SIZEOF_LONG 4			// 64 bit build
#else
#define SIZEOF_LONG 4			// 32 bit build
#endif

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.0.7"

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */
