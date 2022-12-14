/*
 * GNU Universal packer.
 *
 *     General includes.
 *
 * $Author: wout $
 * $Date: 2000-09-03 14:37:02 +0200 (Sun, 03 Sep 2000) $
 * $Revision: 204 $
 * $Log$
 * Revision 1.4  2000/09/03 12:37:02  wout
 * Updated list with OS ID's.
 *
 * Revision 1.3  2000/07/30 15:46:55  wout
 * Added OS id OS_WIN32. Added 64 bit integer support.
 *
 * Revision 1.2  2000/07/16 17:12:40  hwessels
 * Updated to GUP 0.0.4
 *
 * Revision 1.1  1998/12/27 20:48:08  klarenw
 * Removed settings.h. New names for error codes.
 */

#ifndef __GUP_H__
#define __GUP_H__


/*
 * OS id's.
 */

#define OS_MSDOS		0				/* ARJ, LHARC */
#define OS_PRIMOS		1				/* ARJ */
#define OS_UNIX			2				/* ARJ, LHARC */
#define OS_AMIGA		3				/* ARJ, LHARC */
#define OS_MAC			4				/* ARJ, LHARC */
#define OS_OS_2			5				/* ARJ, LHARC */
#define OS_APPLE_GS		6				/* ARJ */
#define OS_ATARI		7				/* ARJ, LHARC */
#define OS_NEXT			8				/* ARJ */
#define OS_VAX_VMS		9				/* ARJ */
#define OS_WIN32		10				/* ARJ */
#define OS_GENERIC		11				/* LHARC */
#define OS_OS9			12				/* LHARC */
#define OS_OS_68K		13				/* LHARC */
#define OS_OS_386		14				/* LHARC */
#define OS_HUMAN		15				/* LHARC */
#define OS_CP_M			16				/* LHARC */
#define OS_FLEX			17				/* LHARC */
#define OS_RUNSER		18				/* LHARC */
#define OS_TOWNOS		19				/* LHARC */
#define OS_XOSK			20				/* LHARC */

#define MAX_OS			OS_XOSK



#if defined(_MSC_VER)
#include "../win32/msvc20xx-common/config.h"
#else
#include "config.h"
#endif

#if defined(_MSC_VER)
#include <crtdbg.h>
#endif
#if (OS == OS_WIN32)
#include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_DIR_H
#include <dir.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif
#if (OS == OS_UNIX)
#include <grp.h>
#include <pwd.h>
#endif
#if defined(HAVE_TOS_H) || (OS == OS_ATARI)
#include <tos.h>
#endif
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>

#include "compiler_intrinsics.h"



// fixes for MSVC et al:
#undef small
#undef large



#if !defined(FALSE) && !defined(TRUE)

#define FALSE			0
#define TRUE			1

#endif

/*
 * Macro preventing waring never used variable foo in function bar.
 */

#define NEVER_USE(x) (void) x

#if defined(HAVE_INTS_H)
#include <ints.h>
#endif

/*
 * Types with known size.
 */

#ifndef HAVE_UINT8
#include <limits.h>
#if CHAR_BIT == 8
#ifndef HAVE_UINT8
typedef unsigned char uint8;
#endif
#ifndef HAVE_INT8
typedef signed char int8;
#endif
#else
#error CHAR_BIT should be 8 for arjbeta to work.
#endif
#endif

#ifndef HAVE_UINT16
#if SIZEOF_SHORT == 2
#ifndef HAVE_UINT16
typedef unsigned short uint16;
#endif
#ifndef HAVE_INT16
typedef short int16;
#endif
#elif SIZEOF_INT == 2
#ifndef HAVE_UINT16
typedef unsigned int uint16;
#endif
#ifndef HAVE_INT16
typedef int int16;
#endif
#else
#error sizeof(short) or sizeof(int) should be 2 for arjbeta to work.
#endif
#endif

#if SIZEOF_INT == 4
#ifndef HAVE_UINT32
typedef unsigned int uint32;
#endif
#ifndef HAVE_INT32
typedef int int32;
#endif
#elif SIZEOF_LONG == 4
#ifndef HAVE_UINT32
typedef unsigned long uint32;
#endif
#ifndef HAVE_INT32
typedef long int32;
#endif
#else
#error sizeof(int) or sizeof(long) should be 4 for arjbeta to work.
#endif

#if SIZEOF_LONG == 8
#ifndef HAVE_UINT64
typedef unsigned long uint64;
#endif
#ifndef HAVE_INT64
typedef long int64;
#endif
#elif defined(HAVE_LONG_LONG) && (SIZEOF_LONG_LONG == 8)
#ifndef HAVE_UINT64
typedef unsigned long long uint64;
#endif
#ifndef HAVE_INT64
typedef long long int64;
#endif
#else
#ifdef HAVE_LONG_LONG
#error sizeof(long) or sizeof(long long) should be 8 for arjbeta to work.
#endif
#endif

/*
 * Support for optimized code/functions.
 */

#ifdef USE_OPTIMIZED
#if (OS == OS_ATARI)
#include "../compress/st_opti/opti.h"
#else
#include "opti.h"
#endif
#endif

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif



#define ENABLE_DUMP_OUTPUT_MODES  1


#ifdef __cplusplus
extern "C" {
#endif

extern FILE* assert_redir_fptr;
int report_assertion_failure(const char* msg, ...);

#ifdef __cplusplus
}
#endif

#if !defined(__PRETTY_FUNCTION__) && defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define __ARJ_Assert(expr)										\
  ((void)((expr) || report_assertion_failure(					\
  "\nAssertion failed: %s, file %s, line %d, function %s\n",	\
   #expr, __FILE__, __LINE__, __PRETTY_FUNCTION__)))


#ifndef NDEBUG

#define ARJ_Assert(expr)   __ARJ_Assert(expr)

#else

#define ARJ_Assert(expr)

#endif


#if 0

#define TRACE_ME()				\
	fprintf(stderr, "%s(%d): %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)

#define TRACE_ME_EX(...)																									\
	do {																															\
		char msgbuf[2048];																									\
		snprintf(msgbuf, sizeof(msgbuf), __VA_ARGS__);																\
		msgbuf[sizeof(msgbuf) - 1] = 0;																					\
		fprintf(stderr, "%s(%d): %s ----> %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, msgbuf);	\
	} while(0)

#else

#define TRACE_ME()

#define TRACE_ME_EX(...)

#endif

#endif
