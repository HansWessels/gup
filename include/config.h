#ifdef _MSC_VER
#error "SHOULD NOT be included when compiling in Microsoft Visual Studio"
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
#define OS OS_UNIX

/* System gup is build for. */
#define SYSTEM ""

/* Define if ints.h exists. */
/* #undef HAVE_INTS_H */

/* Define if the the long long type exists. */
#define HAVE_LONG_LONG 1

/* Define if the the int8 type exists. */
/* #undef HAVE_INT8 */

/* Define if the the uint8 type exists. */
/* #undef HAVE_UINT8 */

/* Define if the the int16 type exists. */
/* #undef HAVE_INT16 */

/* Define if the the uint16 type exists. */
/* #undef HAVE_UINT16 */

/* Define if the the int32 type exists. */
/* #undef HAVE_INT32 */

/* Define if the the uint32 type exists. */
/* #undef HAVE_UINT32 */

/* Define if the the int64 type exists. */
/* #undef HAVE_INT64 */

/* Define if the the uint64 type exists. */
/* #undef HAVE_UINT64 */

/* dmalloc library. */
/* #undef WITH_DMALLOC */

/* _POSIX_C_SOURCE. */
/* #undef _POSIX_C_SOURCE */

/* __EXTENSIONS__. */
/* #undef __EXTENSIONS__ */

#define HAVE_DIRECT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `getgrgid_r' function. */
#define HAVE_GETGRGID_R 1

/* Define to 1 if you have the `getgrnam_r' function. */
#define HAVE_GETGRNAM_R 1

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getpwnam_r' function. */
#define HAVE_GETPWNAM_R 1

/* Define to 1 if you have the `getpwuid_r' function. */
#define HAVE_GETPWUID_R 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

#undef HAVE_MKDIR 
#define HAVE__MKDIR 1

/* Define to 1 if you have the <io.h> header file. */
/* #undef HAVE_IO_H */

/* Define to 1 if you have the `lchown' function. */
#define HAVE_LCHOWN 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have a working `mmap' system call. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <regex.h> header file. */
#define HAVE_REGEX_H 1

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
/* #undef HAVE_STRLWR */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

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
#define SIZEOF_LONG 8

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
